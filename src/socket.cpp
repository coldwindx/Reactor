#include "socket.hpp"
#include <unistd.h>
#include <netinet/tcp.h>

Socket::~Socket()
{
    ::close(fd_);
}

int Socket::fd() const
{
    return fd_;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}

void Socket::setTcpNodelay(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof(optval))); // 必须
}

void Socket::bind(const InetAddress &servaddr)
{
    if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0)
    {
        printf("%s:%s:%d socket bind error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        ::close(fd_);
        exit(-1);
    }
    setIpAndPort(servaddr.ip(), servaddr.port());
    ip_ = servaddr.ip();
    port_ = servaddr.port();
}

void Socket::listen(int nn)
{
    if (::listen(fd_, nn) != 0)
    {
        printf("%s:%s:%d socket listen error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        ::close(fd_);
        exit(-1);
    }
}

int Socket::accept(InetAddress &clientaddr)
{
    sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    int clientfd = accept4(fd_, (sockaddr *)&peeraddr, &len, SOCK_NONBLOCK);
    clientaddr.setaddr(peeraddr);
    return clientfd;
}

void Socket::setIpAndPort(const std::string &ip, uint16_t port)
{
    ip_ = ip;
    port_ = port;
}

int createNoblocking()
{
    // 创建服务器监听的listen fd
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (listenfd < 0)
    {
        printf("%s:%s:%d listen socket create error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    return listenfd;
}
