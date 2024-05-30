#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool
{
public:
    ThreadPool(size_t threadnum, const std::string &threadtype);
    void addtask(std::function<void()> task);
    ~ThreadPool();
    size_t size() const { return threads_.size(); }
    void stop();
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> taskqueue_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic_bool stop_;
    std::string threadtype_;
};