#include "connection.hpp"
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>

Connection::Connection(EventLoop *loop, std::unique_ptr<Socket> clientsock)
    : loop_(loop), clientsock_(std::move(clientsock)), disconnect_(false),
      clientchanel_(new Channel(loop_, clientsock_->fd()))
{
    clientchanel_->setReadCallback(std::bind(&Connection::onMessage, this));
    clientchanel_->setWriteCallback(std::bind(&Connection::writeCallback, this));
    clientchanel_->setCloseCallback(std::bind(&Connection::closeCallback, this));
    clientchanel_->setErrorCallback(std::bind(&Connection::errorCallback, this));
    // clientchanel_->useEt(); // 客户端使用边缘触发
    clientchanel_->enableReading();
}

Connection::~Connection()
{
    // printf("Connection release!\n");
}

void Connection::onMessage()
{
    char buf[4096];
    while (true)
    {
        memset(buf, 0, sizeof(buf));
        ssize_t nread = read(clientsock_->fd(), buf, sizeof(buf));
        if (0 < nread)
        {
            inputbuf_.append(buf, nread);
            continue;
        }
        // 全部数据读取完毕
        if (-1 == nread && (EAGAIN == errno || EWOULDBLOCK == errno))
        {
            std::string msg;
            while (true)
            {
                if(false == inputbuf_.pick(msg))
                    break;
                lastatime_ = Timestamp::now();
                // 回调TcpServer::onMessage()
                messagecallback_(shared_from_this(), msg);
            }
            break;
        }
        // 客户端连接已断开
        if (0 == nread)
        {
            closeCallback();
            break;
        }
        // 读取数据的时候，被信号中断，继续读取
        if (-1 == nread && EINTR == errno)
        {
            continue;
        }
    }
}

void Connection::writeCallback()
{
    // printf("Connection::writeCallback() thread is %ld.\n", syscall(SYS_gettid));
    int writen = ::send(fd(), outputbuf_.data(), outputbuf_.size(), 0);
    if (0 < writen)
        outputbuf_.erase(0, writen);
    // 发送缓冲区没有数据时，关闭监听写事件
    if (0 == outputbuf_.size())
    {
        clientchanel_->disableWriting();
        sendcallback_(shared_from_this());
    }
}

void Connection::closeCallback()
{
    disconnect_ = true;
    clientchanel_->removeFromEventLoop();
    closecallback_(shared_from_this());
}

void Connection::errorCallback()
{
    disconnect_ = true;
    clientchanel_->removeFromEventLoop();
    errorcallback_(shared_from_this());
}

void Connection::send(const std::string &msg, size_t size)
{
    if (disconnect_)
    {
        // printf("Client connection has disconnected.\n");
        return;
    }
    if (loop_->isInLoopThread())
        return syncSend(msg, size);

    loop_->addTask(std::bind(&Connection::syncSend, this, msg, size));
}

void Connection::syncSend(const std::string &msg, size_t size)
{
    // printf("Thread %ld: %d > %s\n", syscall(SYS_gettid), msg, msg.c_str());
    outputbuf_.appendWithSep(msg.c_str(), size);
    clientchanel_->enableWriting(); // 注册写事件，Channel::handle的写事件将被触发
}

bool Connection::timeout(time_t now, int val)
{
    return val < (now - lastatime_.toint());
}
