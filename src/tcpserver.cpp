#include "tcpserver.hpp"
#include "connection.hpp"

TcpServer::TcpServer(const std::string &ip, const uint16_t port, int threadnum)
    : threadnum_(threadnum), mainloop_(new EventLoop(true)), acceptor_(mainloop_.get(), ip, port), threadpool_(threadnum_, "IO")
{

    mainloop_->setEpollTimoutCallback(std::bind(&TcpServer::onEpollTimeout, this, std::placeholders::_1));

    acceptor_.setCallback(std::bind(&TcpServer::onConnect, this, std::placeholders::_1));

    // Create sub loop
    for (int i = 0; i < threadnum_; ++i)
    {
        subloops_.emplace_back(new EventLoop(false, 5, 10));
        subloops_[i]->setEpollTimoutCallback(std::bind(&TcpServer::onEpollTimeout, this, std::placeholders::_1));
        subloops_[i]->setTiemrCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[i].get()));
    }
}

void TcpServer::stop()
{
    mainloop_->stop();
    printf("MainLoop has stoped.\n");

    for (int i = 0; i < threadnum_; ++i)
        subloops_[i]->stop();
    printf("SubLoop has stoped.\n");

    threadpool_.stop();
    
}

void TcpServer::onConnect(std::unique_ptr<Socket> clientsock)
{
    // 新建的conn分配给从事件循环
    Connection::Sptr conn(new Connection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock)));
    conn->setCloseCallback(std::bind(&TcpServer::closeConnect, this, std::placeholders::_1));
    conn->setErrorCallback(std::bind(&TcpServer::errorConnect, this, std::placeholders::_1));
    conn->setMessageCallback(std::bind(&TcpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCallback(std::bind(&TcpServer::sendComplete, this, std::placeholders::_1));

    {
        std::lock_guard<std::mutex> gd(mutex_);
        conns_[conn->fd()] = conn;
    }
    // printf("%d: %ld\n", conn->fd(), conn.use_count());
    subloops_[conn->fd() % threadnum_]->addConnection(conn);
    // 回调函数
    if (connectcallback_)
        connectcallback_(conn);
}

void TcpServer::onMessage(Connection::Sptr conn, std::string message)
{
    if (recvcallback_)
        recvcallback_(conn, message);
}

void TcpServer::sendComplete(Connection::Sptr conn)
{
    if (sendcallback_)
        sendcallback_(conn);
}

void TcpServer::onEpollTimeout(EventLoop *loop)
{
    if (timeoutcallback_)
        timeoutcallback_(loop);
}

void TcpServer::closeConnect(Connection::Sptr conn)
{
    if (closeconnectcallback_)
        closeconnectcallback_(conn);
    // printf("client(fd=%d) disconnected.\n", conn->fd());
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(conn->fd());
}

void TcpServer::errorConnect(Connection::Sptr conn)
{
    if (errorconnectcallback_)
        errorconnectcallback_(conn);
    // printf("client(fd=%d) error.\n", conn->fd());
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(conn->fd());
}

void TcpServer::removeConnection(int fd)
{
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(fd);
}
