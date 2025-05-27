#pragma once
#include <map>
#include <memory>
#include "loop.hpp"
#include "acceptor.hpp"
#include "httpconnection.hpp"
#include "threadpool.hpp"
#include "httprequest.h"
#include "httpresponse.hpp"

using std::string, std::vector, std::shared_ptr, std::function, std::mutex;

// Http网络服务基础类
class HttpServer
{
public:
    HttpServer(const string &ip, const uint16_t port, int porterNum = 3, int workerNum = 5);
    ~HttpServer() = default;

    void start() { mainloop_->run(); } // 运行事件循环
    void stop();

    void connect(unique_ptr<Socket> clientsock);                           // 新的客户端请求，由Acceptor类回调
    void recv(HttpConnection::Sptr conn, shared_ptr<HttpRequest> request); // 客户端请求报文，由Connection类回调
    void send(HttpConnection::Sptr conn, shared_ptr<HttpResponse> response);
    void timeout(EventLoop *loop);         // Epoll事件超时
    void close(HttpConnection::Sptr conn); // 关闭客户端连接，由Connection类回调
    void error(HttpConnection::Sptr conn); // 错误客户端连接，由Connection类回调
    void remove(int fd);                   // 移除客户端连接

    void setConnectCallback(function<void(HttpConnection::Sptr)> callback) { connectcallback_ = callback; }
    void setRecvCallback(function<void(HttpConnection::Sptr, shared_ptr<HttpRequest>)> callback) { recvcallback_ = callback; }
    void setSendCallback(function<void(HttpConnection::Sptr, shared_ptr<HttpResponse>)> callback) { sendcallback_ = callback; }
    void setTimeoutCallback(function<void(EventLoop *)> callback) { timeoutcallback_ = callback; }
    void setCloseCallback(function<void(HttpConnection::Sptr)> callback) { closecallback_ = callback; }
    void setErrorCallback(function<void(HttpConnection::Sptr)> callback) { errorcallback_ = callback; }

private:
    unique_ptr<EventLoop> mainloop_;         // 主事件循环
    vector<unique_ptr<EventLoop>> subloops_; // 从事件循环
    int _porternum, _workernum;              // 线程池大小，即从事件的个数 & 并行请求数
    ThreadPool _porterpool, _workerpool;

    Acceptor acceptor_;

    mutex mutex_;
    map<int, HttpConnection::Sptr> conns_;

    function<void(HttpConnection::Sptr)> connectcallback_;
    function<void(HttpConnection::Sptr, shared_ptr<HttpRequest>)> recvcallback_;
    function<void(HttpConnection::Sptr, shared_ptr<HttpResponse>)> sendcallback_;
    function<void(EventLoop *)> timeoutcallback_;
    function<void(HttpConnection::Sptr)> closecallback_;
    function<void(HttpConnection::Sptr)> errorcallback_;
};