#include "loop.hpp"
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#include <string.h>

int createTiemrFd(int sec = 30)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = sec;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(tfd, 0, &timeout, 0);
    return tfd;
}

EventLoop::EventLoop(bool mainloop, int timetvl, int timeout)
    : mainloop_(mainloop), timetvl_(timetvl), timeout_(timeout), stop_(false),
      ep_(new Epoll),
      wakeupfd_(eventfd(0, EFD_NONBLOCK)),
      wakechannel_(new Channel(this, wakeupfd_)),
      timerfd_(createTiemrFd(timeout)), timerchannel_(new Channel(this, timerfd_))

{
    wakechannel_->setReadCallback(std::bind(&EventLoop::afterWakeup, this));
    wakechannel_->enableReading();

    timerchannel_->setReadCallback(std::bind(&EventLoop::afterTimer, this));
    timerchannel_->enableReading();
}

void EventLoop::run()
{
    threadid_ = syscall(SYS_gettid);
    while (!stop_)
    {
        std::vector<Channel *> channels = ep_->loop();
        if (0 == channels.size())
        {
            epolltimeoutcalback_(this);
            continue;
        }
        for (auto &ch : channels)
            ch->handle();
    }
}

void EventLoop::stop()
{
    stop_ = true;
    // Wakeup event loop
    wakeup();
}

void EventLoop::updatechannel(Channel *ch)
{
    ep_->update(ch);
}

void EventLoop::removechannel(Channel *ch)
{
    ep_->remove(ch);
}

bool EventLoop::isInLoopThread()
{
    return threadid_ == syscall(SYS_gettid);
}

void EventLoop::addTask(std::function<void()> fn)
{
    {
        std::lock_guard<std::mutex> gd(mutex_);
        taskqueue_.push(fn);
    }
    // Wake up the Event loop
    wakeup();
}

void EventLoop::addConnection(std::shared_ptr<Connection> conn)
{
    std::lock_guard<std::mutex> gd(connsmutex_);
    conns_[conn->fd()] = conn;
}

void EventLoop::wakeup()
{
    uint64_t val = 1;
    ::write(wakeupfd_, &val, sizeof(val));
}

void EventLoop::afterWakeup()
{
    // printf("handlewakeup() thread id is %ld.\n", syscall(SYS_gettid));
    uint64_t val;
    read(wakeupfd_, &val, sizeof(val)); // 从eventfd中读取出数据，如果不读取，eventfd的读事件会一直触发。

    std::function<void()> fn;

    std::lock_guard<std::mutex> gd(mutex_); // 给任务队列加锁。

    // 执行队列中全部的发送任务。
    while (taskqueue_.size() > 0)
    {
        fn = std::move(taskqueue_.front()); // 出队一个元素。
        taskqueue_.pop();
        fn(); // 执行任务。
    }
}

void EventLoop::afterTimer()
{
    struct itimerspec timeout;
    memset(&timeout, 0, sizeof(struct itimerspec));
    timeout.it_value.tv_sec = timetvl_;
    timeout.it_value.tv_nsec = 0;
    timerfd_settime(timerfd_, 0, &timeout, 0);

    if (!mainloop_)
    {
        time_t now = time(0);
        for (auto it = conns_.begin(); it != conns_.end();)
        {
            auto [fd, conn] = *it;
            if (conn->timeout(now, timeout_))
            {
                {
                    std::lock_guard<std::mutex> gd(mutex_);
                    it = conns_.erase(it);
                }
                timercallback_(fd);
                continue;
            }
            ++it;
        }
    }
}
