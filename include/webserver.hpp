#pragma once
#include "http/httpserver.hpp"

class WebServer
{
public:
    WebServer(const std::string &ip, const uint16_t port, int reactornum = 3, int workernum = 5);
    ~WebServer() = default;

    void start();
    void stop();

    void connect(HttpConnection::Sptr conn);                // 新的客户端请求，由Acceptor类回调
    void recv(HttpConnection::Sptr conn, shared_ptr<HttpRequest> request); // 客户端请求报文，由Connection类回调
    void send(HttpConnection::Sptr conn, shared_ptr<HttpResponse> response);
    void timeout(EventLoop *loop);
    void close(HttpConnection::Sptr conn); // 关闭客户端连接，由Connection类回调
    void error(HttpConnection::Sptr conn); // 错误客户端连接，由Connection类回调
private:
    HttpServer _server;
    ThreadPool _threadpool;
};