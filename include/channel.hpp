#pragma once
#include <memory>
#include <functional>
#include "socket.hpp"
#include "loop.hpp"

class EventLoop;
// TCP连接通道类
class Channel
{
public:
    Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd) {}
    ~Channel() = default;

    int fd() { return fd_; }
    void useEt();          // 采用边缘触发
    void enableReading();  // 开启epoll_wait()监视fd_的读事件
    void disableReading(); // 关闭epoll_wait()监视fd_的读事件
    void enableWriting();  // 开启epoll_wait()监视fd_的写事件
    void disableWriting(); // 关闭epoll_wait()监视fd_的写事件
    void disableAll();
    void removeFromEventLoop(); // 从事件循环中删除Channel

    void setInepoll() { inepoll_ = true; }
    void setRevents(uint32_t ev) { revents_ = ev; }
    bool inepoll() { return inepoll_; }
    uint32_t events() { return events_; }
    uint32_t revents() { return revents_; }

    void setReadCallback(std::function<void()> callback) { readcallback_ = callback; }
    void setWriteCallback(std::function<void()> callback) { writecallback_ = callback; }
    void setCloseCallback(std::function<void()> callback) { closecallback_ = callback; }
    void setErrorCallback(std::function<void()> callback) { errorcallback_ = callback; }

    void handle(); // 处理epoll_wait()返回的事件

private:
    int fd_; // Channel和fd是一一对应关系
    EventLoop *loop_;
    bool inepoll_ = false; // Channel是否已经添加到Epoll树上
    uint32_t events_ = 0;  // fd_需要监听的事件
    uint32_t revents_ = 0; // fd_需要响应的事件

    std::function<void()> readcallback_;  // fd读事件的回调函数
    std::function<void()> writecallback_; // fd写事件，回调Connection::writeCallback()
    std::function<void()> closecallback_; // fd关闭，回调Connection::closeCallback()
    std::function<void()> errorcallback_; // fd错误，回调Connection::errorCallback()
};