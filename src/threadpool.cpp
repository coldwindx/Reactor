#include "threadpool.hpp"
#include <unistd.h>
#include <sys/syscall.h>

ThreadPool::ThreadPool(size_t threadnum, const std::string &threadtype) : stop_(false), threadtype_(threadtype)
{
    for (size_t i = 0; i < threadnum; ++i)
    {
        threads_.emplace_back([this]()
                              {
            // printf("create %s thread(%ld).\n", threadtype_.c_str(), syscall(SYS_gettid));
            while(false == stop_){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->mutex_);
                    this->condition_.wait(lock, [this](){
                        return (this->stop_) || (!this->taskqueue_.empty());
                    });

                    if(this->stop_ && this->taskqueue_.empty()) return;

                    task = std::move(this->taskqueue_.front());
                    this->taskqueue_.pop();
                }
                // printf("thread is %ld.\n", syscall(SYS_gettid));
                task();     // 执行任务
            } });
    }
}

void ThreadPool::addtask(std::function<void()> task)
{
    {
        std::unique_lock<std::mutex> lock(this->mutex_);
        taskqueue_.push(task);
    }
    condition_.notify_one();
}

ThreadPool::~ThreadPool()
{
    this->stop();
}

void ThreadPool::stop()
{
    if (stop_)
        return;
    stop_ = true;
    condition_.notify_all();
    for (auto &th : threads_)
        th.join();
}
