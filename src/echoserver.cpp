#include "echoserver.hpp"
#include <iostream>
#include <unistd.h>
#include <sys/syscall.h>

EchoServer::EchoServer(const std::string &ip, const uint16_t port, int reactornum, int workernum)
    : tcpserver_(ip, port, reactornum), threadpool_(workernum, "WORKS")
{
    tcpserver_.setConnectCallback(std::bind(&EchoServer::handleConnect, this, std::placeholders::_1));
    tcpserver_.setSendCallback(std::bind(&EchoServer::handleSend, this, std::placeholders::_1));
    tcpserver_.setRecvCallback(std::bind(&EchoServer::handleRecv, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_.setTimeoutCallback(std::bind(&EchoServer::handleTimeout, this, std::placeholders::_1));
    tcpserver_.setCloseConnectionCallback(std::bind(&EchoServer::handleCloseConnection, this, std::placeholders::_1));
    tcpserver_.setErrorConnectionCallback(std::bind(&EchoServer::handleCloseConnection, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

void EchoServer::start()
{
    tcpserver_.start();
}

void EchoServer::stop()
{
    // 1. Stop worker threads
    threadpool_.stop();
    // 2. Stop IO threads
    tcpserver_.stop();
}

void EchoServer::handleConnect(Connection::Sptr conn)
{
    printf("%s new connection(fd=%d, ip=%s, port=%d) ok.\n",
           Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
}

void EchoServer::handleRecv(Connection::Sptr conn, std::string &message)
{
    // printf("EchoServer::handleRecv() thread is %ld.\n", syscall(SYS_gettid));
    if (0 == threadpool_.size())
    {
        onRecv(conn, message);
    }
    else
    {
        // 业务添加到worker threadpool 中
        threadpool_.addtask(std::bind(&EchoServer::onRecv, this, conn, message));
    }
}

void EchoServer::handleSend(Connection::Sptr conn)
{
    // std::cout << "Message send." << std::endl;
}

void EchoServer::handleTimeout(EventLoop *loop)
{
    // std::cout << "Epoll timeout." << std::endl;
}

void EchoServer::handleCloseConnection(Connection::Sptr conn)
{
    printf("%s close connection(fd=%d, ip=%s, port=%d) ok.\n",
           Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
}

void EchoServer::handleErrorConnection(Connection::Sptr conn)
{
    // std::cout << "Connection error." << std::endl;
}

void EchoServer::onRecv(Connection::Sptr conn, std::string &message)
{
    // printf("%s message(%d): %s.\n",
    //        Timestamp::now().tostring().c_str(), conn->fd(), message.c_str());
    message = "reply: " + message;
    // 这里如果conn被reactor线程释放，出现野指针 ---> 智能指针
    conn->send(message.data(), message.size());
}
