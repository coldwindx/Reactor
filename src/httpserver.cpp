#include "http/httpserver.hpp"

HttpServer::HttpServer(const std::string &ip, const uint16_t port, int porterNum, int workerNum)
    : _porternum(porterNum), _workernum(workerNum), mainloop_(new EventLoop(true)), acceptor_(mainloop_.get(), ip, port),
      _porterpool(_porternum, "IO"), _workerpool(_workernum, "WORKER")
{

    mainloop_->setEpollTimoutCallback(std::bind(&HttpServer::timeout, this, std::placeholders::_1));
    acceptor_.setCallback(std::bind(&HttpServer::connect, this, std::placeholders::_1));

    // Create sub loop
    for (int i = 0; i < _porternum; ++i)
    {
        subloops_.emplace_back(new EventLoop(false, 5, 10));
        subloops_[i]->setEpollTimoutCallback(std::bind(&HttpServer::timeout, this, std::placeholders::_1));
        subloops_[i]->setTimerCallback(std::bind(&HttpServer::remove, this, std::placeholders::_1));
        _porterpool.addtask(std::bind(&EventLoop::run, subloops_[i].get()));
    }
}

void HttpServer::stop()
{
    mainloop_->stop();
    printf("MainLoop has stoped.\n");

    for (int i = 0; i < _porternum; ++i)
        subloops_[i]->stop();
    printf("SubLoop has stoped.\n");

    _porterpool.stop();
    _workerpool.stop();
}

void HttpServer::connect(std::unique_ptr<Socket> clientsock)
{
    // 新建的conn分配给从事件循环
    HttpConnection::Sptr conn(new HttpConnection(subloops_[clientsock->fd() % _porternum].get(), std::move(clientsock)));
    conn->setCloseCallback(std::bind(&HttpServer::close, this, std::placeholders::_1));
    conn->setErrorCallback(std::bind(&HttpServer::error, this, std::placeholders::_1));
    conn->setRecvCallback(std::bind(&HttpServer::recv, this, std::placeholders::_1, std::placeholders::_2));
    conn->setSendCallback(std::bind(&HttpServer::send, this, std::placeholders::_1, std::placeholders::_2));

    {
        std::lock_guard<std::mutex> gd(mutex_);
        conns_[conn->fd()] = conn;
    }
    // printf("%d: %ld\n", conn->fd(), conn.use_count());
    subloops_[conn->fd() % _porternum]->addConnection(conn);
    // 回调函数
    if (connectcallback_)
        connectcallback_(conn);
}

void HttpServer::recv(HttpConnection::Sptr conn, shared_ptr<HttpRequest> request)
{

    auto toSend = [&]()
    {
        shared_ptr<HttpResponse> response = std::make_shared<HttpResponse>();
        response->setVersion("HTTP/1.0");
        response->setCode("200");
        response->setDescription("OK");
        response->setBody("\
<!DOCTYPE html> \
<html> \
<head> \
	<meta charset=\"utf-8\"> \
	<title>锤子在线工具(toolhelper.cn)</title> \
</head> \
<body> \
	<p id=\"demo\">单击按钮显示指定 UTF-8 编码的字符。</p> \
	<button onclick=\"myFunction()\">点我</button> \
	<script> \
		function myFunction(){ \
			var n=String.fromCharCode(72,69,76,76,79); \
			document.getElementById(\"demo\").innerHTML=n; \
		} \
	</script> \
</body> \
</html>");
        response->addHeader("Server", "Web Server");
        response->addHeader("Content-Type", "text/html");
        response->addHeader("Connection", "Close");
        // response.addHeader("Content-Length", std::to_string(response.getBody().size()));
        // TODO: 这里如果conn被reactor线程释放，出现野指针 ---> 智能指针
        conn->send(response);
    };

    // when 业务线程不存在时 then 主线程负责处理
    if (0 == _workerpool.size())
        return toSend();

    // else 业务添加到worker threadpool 中
    _workerpool.addtask(std::move(toSend));
}

void HttpServer::send(HttpConnection::Sptr conn, shared_ptr<HttpResponse> response)
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
