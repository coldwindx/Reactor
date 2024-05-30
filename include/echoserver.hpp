#pragma once
#include "tcpserver.hpp"

class EchoServer
{
public:
    EchoServer(const std::string &ip, const uint16_t port, int reactornum = 3, int workernum = 5);
    ~EchoServer();

    void start();
    void stop();

    void handleConnect(Connection::Sptr conn);                // 新的客户端请求，由Acceptor类回调
    void handleRecv(Connection::Sptr conn, std::string &msg); // 客户端请求报文，由Connection类回调
    void handleSend(Connection::Sptr conn);
    void handleTimeout(EventLoop *loop);
    void handleCloseConnection(Connection::Sptr conn); // 关闭客户端连接，由Connection类回调
    void handleErrorConnection(Connection::Sptr conn); // 错误客户端连接，由Connection类回调

    void onRecv(Connection::Sptr conn, std::string &msg);
private:
    TcpServer tcpserver_;
    ThreadPool threadpool_;
};