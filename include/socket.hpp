#pragma once
#include "address.h"

// 创建非阻塞Socket
int createNoblocking();

class Socket
{
public:
    Socket(int fd) : fd_(fd) {}
    ~Socket();

    int fd() const;
    void setReuseAddr(bool on);  // 设置SO_REUSEADDR选项
    void setReusePort(bool on);  // 设置SO_REUSEPORT选项
    void setTcpNodelay(bool on); // 设置TCP_NODELAY选项
    void setKeepAlive(bool on);  // 设置SO_KEEP_ALIVE选项

    void bind(const InetAddress &servaddr);
    void listen(int nn = 128); // 高并发网络服务器中，第二个参数要更大一些
    int accept(InetAddress &clientaddr);

    std::string ip() const { return ip_; }
    uint16_t port() const { return port_; }
    void setIpAndPort(const std::string & ip, uint16_t port);

private:
    const int fd_;
    std::string ip_;
    uint16_t port_;
};
