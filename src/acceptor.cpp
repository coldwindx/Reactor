#include "acceptor.hpp"
#include "connection.hpp"
#include <string.h>
#include <unistd.h>

Acceptor::Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port)
    : loop_(loop), servsock_(createNoblocking()), acceptchanel_(loop_, servsock_.fd())
{
    InetAddress servaddr(ip, port);
    servsock_.setReuseAddr(true);
    servsock_.setReusePort(true);
    servsock_.setTcpNodelay(true);
    servsock_.setKeepAlive(true);
    servsock_.bind(servaddr);
    servsock_.listen();

    acceptchanel_.setReadCallback(std::bind(&Acceptor::onConnect, this));
    acceptchanel_.enableReading();
}

Acceptor::~Acceptor()
{
}

void Acceptor::onConnect()
{
    InetAddress clientaddr;
    // clientsock只能new出來，不能放到栈上，否则析构时clientfd会被释放
    std::unique_ptr<Socket> clientsock(new Socket(servsock_.accept(clientaddr)));
    clientsock->setIpAndPort(clientaddr.ip(), clientaddr.port());
    callback_(std::move(clientsock));
}

void Acceptor::setCallback(std::function<void(std::unique_ptr<Socket>)> callback)
{
    callback_ = callback;
}
