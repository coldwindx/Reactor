#include "ctcp.h"
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fstream>

CtcpClient::~CtcpClient()
{
    this->close();
}

bool CtcpClient::connect(const std::string &ip, const unsigned short port)
{
    // Socket已连接
    if (-1 != m_clientfd)
        return false;
    // 创建Socket
    m_clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_clientfd)
    {
        perror("create client socket error!\n");
        return false;
    }

    // 获取服务端网络通信地址
    this->m_ip = ip;
    this->m_port = port;
    struct hostent *h = gethostbyname(m_ip.c_str());
    if (h == nullptr)
    {
        perror("host is not found!\n");
        ::close(m_clientfd);
        m_clientfd = -1;
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr, h->h_addr, h->h_length);
    addr.sin_port = htons(m_port);
    // Connet Server
    int res = ::connect(m_clientfd, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == res)
    {
        perror("connect remote server fail!\n");
        ::close(m_clientfd);
        m_clientfd = -1;
        return false;
    }
    return true;
}

bool CtcpClient::recv(std::string &buf, const int maxlen)
{
    buf.clear();
    buf.resize(maxlen);
    // ::recv自动检测socket连接状况
    int res = ::recv(m_clientfd, &buf[0], buf.size(), 0);
    if (res <= 0)
    {
        buf.clear();
        return false;
    }
    buf.resize(res);
    return true;
}

bool CtcpClient::send(const std::string &buf)
{
    if (-1 == m_clientfd)
        return false;
    int res = ::send(m_clientfd, buf.data(), buf.size(), 0);
    return 0 < res;
}

bool CtcpClient::send(void *buf, const size_t size)
{
    if (-1 == m_clientfd)
        return false;
    int res = ::send(m_clientfd, buf, size, 0);
    return 0 < res;
}

bool CtcpClient::fsend(const std::string &filename, const size_t filesize)
{
    if (-1 == m_clientfd)
        return false;
    std::ifstream fin(filename, std::ios::binary);
    if(!fin.is_open()){
        std::string err = "cannot open file " + filename + "\n";
        perror(err.c_str());
        return false;
    }

    int onread = 0, total = 0;
    char buf[4096];

    while(total < filesize){
        memset(buf, 0, sizeof(buf));

        if(4096 < filesize - total){
            onread = 4096;
        }else{
            onread = filesize - total;
        }
        fin.read(buf, onread);
        int res = ::send(m_clientfd, buf, onread, 0);
        if(res <= 0){
            perror("send file data error!\n");
            return false;
        }
        
        total += onread;
    }
    return true;
}

bool CtcpClient::close()
{
    if (-1 == m_clientfd)
        return false;
    ::close(m_clientfd);
    m_clientfd = -1;
    return true;
}

CtcpServer::~CtcpServer()
{
    closeclient();
    closelisten();
}

bool CtcpServer::initserver(const unsigned short port)
{
    if (-1 != m_listenfd)
        return false;
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_listenfd)
    {
        perror("create listen socket error!\n");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int res = ::bind(m_listenfd, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == res)
    {
        perror("bind socket with address fail!\n");
        ::close(m_listenfd);
        m_listenfd = -1;
        return false;
    }

    res = listen(m_listenfd, 5);
    if (-1 == res)
    {
        perror("listen socket fail!\n");
        ::close(m_listenfd);
        m_listenfd = -1;
        return false;
    }
    return true;
}

bool CtcpServer::accept()
{
    if(-1 == m_listenfd){
        perror("listen socket had closed!\n");
        return false;
    }
    struct sockaddr_in caddr;
    socklen_t addrlen = sizeof(caddr);
    m_clientfd = ::accept(m_listenfd, (struct sockaddr *)&caddr, &addrlen);
    if (-1 == m_clientfd)
    {
        perror("accept client connection request fail!\n");
        return false;
    }
    m_clientip = inet_ntoa(caddr.sin_addr);
    return true;
}

bool CtcpServer::recv(std::string &buf, const int maxlen)
{
    buf.clear();
    buf.resize(maxlen);
    // ::recv自动检测socket连接状况
    int res = ::recv(m_clientfd, &buf[0], buf.size(), 0);
    if (res <= 0)
    {
        buf.clear();
        return false;
    }
    buf.resize(res);
    return true;
}

bool CtcpServer::recv(void * buf, const size_t size)
{
    int res = ::recv(m_clientfd, buf, size, 0);
    return 0 < res;
}

bool CtcpServer::frecv(const std::string &filename, const size_t filesize)
{
    std::ofstream fout(filename, std::ios::binary);
    if(!fout.is_open()){
        std::string err = "cannot open file " + filename + "\n";
        perror(err.c_str());
        return false;
    }

    int onwrite = 0, total = 0;
    char buf[4096];
    while(total < filesize){
        memset(buf, 0, sizeof(buf));
        if(4096 < filesize - total)
            onwrite = 4096;
        else
            onwrite = filesize - total;
        int res = ::recv(m_clientfd, buf, onwrite, 0);
        if(res < 0){
            perror("recv file data error!\n");
            return false;
        }
        fout.write(buf, onwrite);
        total += onwrite;
    }
    return true;
}

bool CtcpServer::send(const std::string &buf)
{
    if (-1 == m_clientfd)
        return false;
    int res = ::send(m_clientfd, buf.data(), buf.size(), 0);
    return 0 < res;
}

bool CtcpServer::closelisten()
{
    if (-1 == m_listenfd)
        return false;
    ::close(m_listenfd);
    m_listenfd = -1;
    return true;
}

bool CtcpServer::closeclient()
{
    if (-1 == m_clientfd)
        return false;
    ::close(m_clientfd);
    m_clientfd = -1;
    return true;
}
