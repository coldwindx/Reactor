#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <string>
#include "http/httpconnection.hpp"

using std::string;

thread_local int errorno = COMPLETED;

HttpConnection::HttpConnection(EventLoop *loop, std::unique_ptr<Socket> clientsock)
    : loop_(loop), clientsock_(std::move(clientsock)), disconnect_(false),
      clientchanel_(new Channel(loop_, clientsock_->fd()))
{
    clientchanel_->setReadCallback(std::bind(&HttpConnection::readCallack, this));
    clientchanel_->setWriteCallback(std::bind(&HttpConnection::writeCallback, this));
    clientchanel_->setCloseCallback(std::bind(&HttpConnection::closeCallback, this));
    clientchanel_->setErrorCallback(std::bind(&HttpConnection::errorCallback, this));
    // clientchanel_->useEt(); // 客户端使用边缘触发
    clientchanel_->enableReading();
}

int HttpConnection::readline(int fd, char *buf, int size)
{
    char ch = '\0';
    int count = 0;
    errorno = TERMINATION;

    while ((count < size - 1) && ch != '\n')
    {
        int n = read(fd, &ch, 1);
        // case 1: 全部数据读取完毕
        if (-1 == n && (EAGAIN == errno || EWOULDBLOCK == errno))
        {
            errorno = COMPLETED;
            return count;
        }
        // case 2: 客户端连接已断开
        if (n == 0)
        {
            errorno = CLOSED;
            return count;
        }
        // case 3: 读取数据的时候，被信号中断，继续读取
        if (-1 == n && EINTR == errno)
            continue;
        // case 3: 正常读取
        if (ch == '\r')
            continue;
        if (ch == '\n')
            break;
        buf[count++] = ch;
    }
    return count;
}

void HttpConnection::readCallack()
{
    int len = 0;
    char buf[4096];
    // 1. 读取http请求行
    memset(buf, 0, sizeof(buf));
    len = readline(clientsock_->fd(), buf, sizeof(buf));
    if (CLOSED == errorno)
        return this->closeCallback();

    char method[64], url[256], version[64];
    memset(method, 0, sizeof(method));
    memset(url, 0, sizeof(url));
    memset(version, 0, sizeof(version));
    string body;
    // 1.1 读取请求类型
    int j = 0;
    for (int i = 0; !isspace(buf[j]) && i < sizeof(method) - 1; ++i, ++j)
        method[i] = buf[j];
    // if (0 == strncasecmp(method, "GET", sizeof(method)))
    while (isspace(buf[j]))
        ++j; // 跳过白空格
    // 1.2 读取URL部分
    for (int i = 0; !isspace(buf[j]) && i < sizeof(url) - 1; ++i, ++j)
        url[i] = buf[j];
    // 1.3 读取VERSION部分
    while (isspace(buf[j]))
        ++j; // 跳过白空格
    for (int i = 0; !isspace(buf[j]) && i < sizeof(url) - 1; ++i, ++j)
        version[i] = buf[j];
    // 2 读取请求头部
    do
    {
        memset(buf, 0, sizeof(buf));
        len = readline(clientsock_->fd(), buf, sizeof(buf));
    } while (len > 0);
    // 3. 读取body部分
    do
    {
        memset(buf, 0, sizeof(buf));
        len = readline(clientsock_->fd(), buf, sizeof(buf));
        body.append(buf, len);
    } while (TERMINATION == errorno);

    // 4. 组装HttpRequest数据
    shared_ptr<HttpRequest> request = std::make_shared<HttpRequest>();
    request->method = std::move(method);
    request->url = std::move(url);
    request->version = std::move(version);
    request->body = std::move(body);

    lastatime_ = Timestamp::now();
    recvcallback_(shared_from_this(), request);
    if (CLOSED == errorno)
        this->closeCallback();
}

void HttpConnection::writeCallback()
{
    // 1. 从数据缓冲区获取要发送的数据
    shared_ptr<HttpResponse> response = std::static_pointer_cast<HttpResponse>(_storage);
    _storage.reset();

    int len = 0;
    // 2. 发送响应头
    string buf = response->version + " " +
          response->code + " " +
          response->description + "\r\n";
    for (auto &[k, v] : response->headers)
        buf += k + ":" + v + "\r\n";

    len = ::send(fd(), buf.data(), buf.size(), 0);
    buf.clear();
    // 2. 发送Content-Length
    buf = "Content-Length: " + std::to_string(strlen(response->body.c_str())) + "\r\n\r\n";
    len = ::send(fd(), buf.data(), buf.size(), 0);
    buf.clear();
    // 3. 发送数据体
    len = ::send(fd(), response->body.data(), response->body.size(), 0);

    // 4. 关闭监听写事件
    clientchanel_->disableWriting();
    // sendcallback_(shared_from_this());
}

void HttpConnection::closeCallback()
{
    disconnect_ = true;
    clientchanel_->removeFromEventLoop();
    closecallback_(shared_from_this());
}

void HttpConnection::errorCallback()
{
    disconnect_ = true;
    clientchanel_->removeFromEventLoop();
    errorcallback_(shared_from_this());
}

void HttpConnection::send(shared_ptr<HttpResponse> response)
{
    if (disconnect_)
        return;
    auto _send = [&]()
    {
        this->_storage = response;      // 将数据移交给缓冲区
        clientchanel_->enableWriting(); // 注册写事件，Channel::handle的写事件将被触发
    };
    if (loop_->isInLoopThread())
        return _send();
    loop_->addTask(_send);
}

bool HttpConnection::timeout(time_t now, int val)
{
    return val < (now - lastatime_.toint());
}
