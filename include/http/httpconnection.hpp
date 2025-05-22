#pragma once
#include <memory>
#include <atomic>
#include "loop.hpp"
#include "timestamp.hpp"
#include "channel.hpp"
#include "httprequest.hpp"
#include "httpresponse.hpp"

using std::string, std::unique_ptr, std::shared_ptr, std::atomic, std::function;

class EventLoop;
class Channel;

#define COMPLETED 0   /* 完成 */
#define TERMINATION 1 /* 中断 */
#define CLOSED 2      /* 断开连接*/

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
public:
    using Sptr = shared_ptr<HttpConnection>;

    HttpConnection(EventLoop *loop, unique_ptr<Socket> clientsock);
    ~HttpConnection() = default;

    int fd() const { return clientsock_->fd(); }
    string ip() const { return clientsock_->ip(); }
    uint16_t port() const { return clientsock_->port(); }

    void readCallack();   // 读事件的回调函数，在此完成Http请求解析
    void writeCallback(); // 写事件的回调函数，nel类回调
    void closeCallback(); // TCP连接断开的回调函数，供Channel类回调
    void errorCallback(); // TCP连接错误的回调函数，供Channel类回调

    void setCloseCallback(function<void(HttpConnection::Sptr)> callback) { closecallback_ = callback; }
    void setErrorCallback(function<void(HttpConnection::Sptr)> callback) { errorcallback_ = callback; }
    void setRecvCallback(function<void(HttpConnection::Sptr, shared_ptr<HttpRequest>)> callback) { recvcallback_ = callback; }
    void setSendCallback(function<void(HttpConnection::Sptr, shared_ptr<HttpResponse>)> callback) { sendcallback_ = callback; }

    void send(shared_ptr<HttpResponse> response);

    bool timeout(time_t now, int val);

protected:
    /// @brief 从fd中读取一行数据
    /// @param fd
    /// @return 0-连接中断，1-完成，-1-错误
    int readline(int fd, char *buf, int size);

private:
    EventLoop *loop_;
    unique_ptr<Socket> clientsock_;
    unique_ptr<Channel> clientchanel_;
    atomic<bool> disconnect_; // 客户端是否已经断开

    // 回调函数
    function<void(HttpConnection::Sptr)> closecallback_;
    function<void(HttpConnection::Sptr)> errorcallback_;
    function<void(HttpConnection::Sptr, shared_ptr<HttpRequest>)> recvcallback_;
    function<void(HttpConnection::Sptr, shared_ptr<HttpResponse>)> sendcallback_;

    Timestamp lastatime_;
    // 数据缓冲区
    shared_ptr<void> _storage;
};