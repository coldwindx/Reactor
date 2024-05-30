#pragma once
#include <map>
#include <memory>
#include "loop.hpp"
#include "acceptor.hpp"
#include "connection.hpp"
#include "threadpool.hpp"

// TCP网络服务类
class TcpServer
{
public:
    TcpServer(const std::string &ip, const uint16_t port, int threadnum = 3);
    ~TcpServer() = default;

    void start() { mainloop_->run(); } // 运行事件循环
    void stop();

    void onConnect(std::unique_ptr<Socket> clientsock);     // 新的客户端请求，由Acceptor类回调
    void onMessage(Connection::Sptr conn, std::string msg); // 客户端请求报文，由Connection类回调
    void sendComplete(Connection::Sptr conn);
    void onEpollTimeout(EventLoop *loop);
    void closeConnect(Connection::Sptr conn); // 关闭客户端连接，由Connection类回调
    void errorConnect(Connection::Sptr conn); // 错误客户端连接，由Connection类回调
    void removeConnection(int fd);

    void setConnectCallback(std::function<void(Connection::Sptr)> callback) { connectcallback_ = callback; }
    void setRecvCallback(std::function<void(Connection::Sptr, std::string &)> callback) { recvcallback_ = callback; }
    void setSendCallback(std::function<void(Connection::Sptr)> callback) { sendcallback_ = callback; }
    void setTimeoutCallback(std::function<void(EventLoop *)> callback) { timeoutcallback_ = callback; }
    void setCloseConnectionCallback(std::function<void(Connection::Sptr)> callback) { closeconnectcallback_ = callback; }
    void setErrorConnectionCallback(std::function<void(Connection::Sptr)> callback) { errorconnectcallback_ = callback; }

private:
    std::unique_ptr<EventLoop> mainloop_;               // 主事件循环
    std::vector<std::unique_ptr<EventLoop>> subloops_; // 从事件循环
    int threadnum_; // 线程池大小，即从事件的个数
    ThreadPool threadpool_;
    
    Acceptor acceptor_;
    
    std::mutex mutex_;
    std::map<int, Connection::Sptr> conns_;

    std::function<void(Connection::Sptr)> connectcallback_;
    std::function<void(Connection::Sptr, std::string &)> recvcallback_;
    std::function<void(Connection::Sptr)> sendcallback_;
    std::function<void(EventLoop *)> timeoutcallback_;
    std::function<void(Connection::Sptr)> closeconnectcallback_;
    std::function<void(Connection::Sptr)> errorconnectcallback_;
};