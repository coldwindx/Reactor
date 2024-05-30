#pragma once
#include <string>

class CtcpClient
{
public:
    CtcpClient() : m_clientfd(-1) {}
    ~CtcpClient();
    bool connect(const std::string &ip, const unsigned short port);
    bool recv(std::string &buf, const int maxlen);

    bool send(const std::string &buf);
    bool send(void * buf, const size_t size);
    bool fsend(const std::string & filename, const size_t filesize);
    bool close();

private:
    int m_clientfd;        // 客户端Socket句柄
    std::string m_ip;      // 服务器IP
    unsigned short m_port; // 服务器Port
};

class CtcpServer
{

public:
    CtcpServer() : m_listenfd(-1), m_clientfd(-1) {}
    ~CtcpServer();

    bool initserver(const unsigned short port);
    bool accept();

    bool recv(std::string &buf, const int maxlen);
    bool recv(void * buf, const size_t size);
    bool frecv(const std::string & filename, const size_t filesize);
    bool send(const std::string &buf);
    bool closelisten();
    bool closeclient();

    std::string clientip() const{
        return m_clientip;
    }
private:
    int m_listenfd;
    int m_clientfd;
    std::string m_clientip;
    unsigned short m_port;
};