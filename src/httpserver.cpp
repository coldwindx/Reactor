#include "http/httpserver.hpp"

HttpServer::HttpServer(const std::string &ip, const uint16_t port, int threadnum)
    : threadnum_(threadnum), mainloop_(new EventLoop(true)), acceptor_(mainloop_.get(), ip, port), threadpool_(threadnum_, "IO")
{

    mainloop_->setEpollTimoutCallback(std::bind(&HttpServer::timeout, this, std::placeholders::_1));
    acceptor_.setCallback(std::bind(&HttpServer::connect, this, std::placeholders::_1));

    // Create sub loop
    for (int i = 0; i < threadnum_; ++i)
    {
        subloops_.emplace_back(new EventLoop(false, 5, 10));
        subloops_[i]->setEpollTimoutCallback(std::bind(&HttpServer::timeout, this, std::placeholders::_1));
        subloops_[i]->setTimerCallback(std::bind(&HttpServer::remove, this, std::placeholders::_1));
        threadpool_.addtask(std::bind(&EventLoop::run, subloops_[i].get()));
    }
}

void HttpServer::stop()
{
    mainloop_->stop();
    printf("MainLoop has stoped.\n");

    for (int i = 0; i < threadnum_; ++i)
        subloops_[i]->stop();
    printf("SubLoop has stoped.\n");

    threadpool_.stop();
}

void HttpServer::connect(std::unique_ptr<Socket> clientsock)
{
    // 新建的conn分配给从事件循环
    HttpConnection::Sptr conn(new HttpConnection(subloops_[clientsock->fd() % threadnum_].get(), std::move(clientsock)));
    conn->setCloseCallback(std::bind(&HttpServer::close, this, std::placeholders::_1));
    conn->setErrorCallback(std::bind(&HttpServer::error, this, std::placeholders::_1));
    conn->setRecvCallback(std::bind(&HttpServer::recv, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCallback(std::bind(&HttpServer::send, this, std::placeholders::_1, std::placeholders::_2));

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

void HttpServer::recv(HttpConnection::Sptr conn, shared_ptr<HttpRequest> request)
{
    if (recvcallback_)
        recvcallback_(conn, request);
}

void HttpServer::send(HttpConnection::Sptr conn,  shared_ptr<HttpResponse> response)
{
    if (sendcallback_)
        sendcallback_(conn, response);
}

void HttpServer::timeout(EventLoop *loop)
{
    if (timeoutcallback_)
        timeoutcallback_(loop);
}

void HttpServer::close(HttpConnection::Sptr conn)
{
    if (closecallback_)
        closecallback_(conn);
    // printf("client(fd=%d) disconnected.\n", conn->fd());
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(conn->fd());
}

void HttpServer::error(HttpConnection::Sptr conn)
{
    if (errorcallback_)
        errorcallback_(conn);
    // printf("client(fd=%d) error.\n", conn->fd());
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(conn->fd());
}

void HttpServer::remove(int fd)
{
    std::lock_guard<std::mutex> gd(mutex_);
    conns_.erase(fd);
}
