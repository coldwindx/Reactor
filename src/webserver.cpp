#include <unistd.h>
#include <sys/syscall.h>
#include <string>
#include "webserver.hpp"
#include "http/httpresponse.hpp"

using std::string;

WebServer::WebServer(const std::string &ip, const uint16_t port, int reactornum, int workernum)
    : _server(ip, port, reactornum), _threadpool(workernum, "WORKS")
{
    _server.setConnectCallback(std::bind(&WebServer::connect, this, std::placeholders::_1));
    _server.setSendCallback(std::bind(&WebServer::send, this, std::placeholders::_1, std::placeholders::_2));
    _server.setRecvCallback(std::bind(&WebServer::recv, this, std::placeholders::_1, std::placeholders::_2));
    _server.setTimeoutCallback(std::bind(&WebServer::timeout, this, std::placeholders::_1));
    _server.setCloseCallback(std::bind(&WebServer::close, this, std::placeholders::_1));
    _server.setErrorCallback(std::bind(&WebServer::error, this, std::placeholders::_1));
}

void WebServer::start()
{
    _server.start();
}

void WebServer::stop()
{
    // 1. Stop worker threads
    _threadpool.stop();
    // 2. Stop IO threads
    _server.stop();
}

void WebServer::connect(HttpConnection::Sptr conn)
{
    printf("%s new connection(fd=%d, ip=%s, port=%d) ok.\n",
           Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
}

void WebServer::recv(HttpConnection::Sptr conn, shared_ptr<HttpRequest> request)
{
    auto _recv = [&]()
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
    if (0 == _threadpool.size())
        return _recv();
    // else 业务添加到worker threadpool 中
    _threadpool.addtask(_recv);
}

void WebServer::send(HttpConnection::Sptr conn, shared_ptr<HttpResponse> response)
{
    // std::cout << "Message send." << std::endl;
}

void WebServer::timeout(EventLoop *loop)
{
    // std::cout << "Epoll timeout." << std::endl;
}

void WebServer::close(HttpConnection::Sptr conn)
{
    printf("%s close connection(fd=%d, ip=%s, port=%d) ok.\n",
           Timestamp::now().tostring().c_str(), conn->fd(), conn->ip().c_str(), conn->port());
}

void WebServer::error(HttpConnection::Sptr conn)
{
    // std::cout << "HttpConnection error." << std::endl;
}