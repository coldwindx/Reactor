#pragma once
#include "epoll.hpp"
#include <sys/eventfd.h>
#include <mutex>
#include <queue>
#include <map>
#include <memory>
#include "connection.hpp"

class Channel;
class Epoll;
class Connection;

// 事件循环类
class EventLoop
{
public:
    EventLoop(bool mainloop, int timetvl = 30, int timeout = 100);
    ~EventLoop() = default;

    void run();
    void stop();

    void updatechannel(Channel *ch);
    void removechannel(Channel *ch);

    void setEpollTimoutCallback(std::function<void(EventLoop *)> callback) { epolltimeoutcalback_ = callback; }
    void setTiemrCallback(std::function<void(int)> callback) { timercallback_ = callback; }

    bool isInLoopThread();

    void addTask(std::function<void()> fn);
    void addConnection(std::shared_ptr<Connection> conn);

    void wakeup();
    void afterWakeup();
    void afterTimer();

private:
    std::unique_ptr<Epoll> ep_;
    std::function<void(EventLoop *)> epolltimeoutcalback_;
    pid_t threadid_; // Thread id of event loop

    std::queue<std::function<void()>> taskqueue_;
    std::mutex mutex_;

    int wakeupfd_;
    std::unique_ptr<Channel> wakechannel_;

    int timerfd_;
    std::unique_ptr<Channel> timerchannel_;

    bool mainloop_;
    std::mutex connsmutex_;
    std::map<int, std::shared_ptr<Connection>> conns_;
    std::function<void(int)> timercallback_;
    int timetvl_;
    int timeout_;

    std::atomic<bool> stop_;
};
