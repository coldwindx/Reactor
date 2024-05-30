#pragma once
#include <memory>
#include <atomic>
#include "loop.hpp"
#include "buffer.hpp"
#include "timestamp.hpp"

class EventLoop;
class Channel;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    using Sptr = std::shared_ptr<Connection>;

    Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock);
    ~Connection();

    int fd() const { return clientsock_->fd(); }
    std::string ip() const { return clientsock_->ip(); }
    uint16_t port() const { return clientsock_->port(); }

    void onMessage();
    void writeCallback(); // 写事件的回调函数，nel类回调
    void closeCallback(); // TCP连接断开的回调函数，供Channel类回调
    void errorCallback(); // TCP连接错误的回调函数，供Channel类回调

    void setCloseCallback(std::function<void(Connection::Sptr)> callback) { closecallback_ = callback; }
    void setErrorCallback(std::function<void(Connection::Sptr)> callback) { errorcallback_ = callback; }
    void setMessageCallback(std::function<void(Connection::Sptr, std::string &)> callback) { messagecallback_ = callback; }
    void setSendCallback(std::function<void(Connection::Sptr)> callback) { sendcallback_ = callback; }

    void send(const std::string &msg, size_t size);
    void syncSend(const std::string &msg, size_t size);

    bool timeout(time_t now, int val);
private:
    EventLoop *loop_;
    std::unique_ptr<Socket> clientsock_;
    std::unique_ptr<Channel> clientchanel_;
    std::atomic<bool> disconnect_; // 客户端是否已经断开

    Buffer inputbuf_;  // 接收缓冲区
    Buffer outputbuf_; // 发送缓冲区

    // TcpServer类的回调函数
    std::function<void(Connection::Sptr)> closecallback_;
    std::function<void(Connection::Sptr)> errorcallback_;
    std::function<void(Connection::Sptr, std::string &)> messagecallback_;
    std::function<void(Connection::Sptr)> sendcallback_;

    Timestamp lastatime_;
};