#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "address.h"
#include "socket.hpp"
#include "epoll.hpp"
#include "loop.hpp"
#include "http/httpserver.hpp"
#include "http/httpmethod.h"
#include "web/mapping.h"
#include "web/servlet.h"
#include <iostream>
using namespace std;

class Conroller
{
public:
    string hello(string_view name)
    {
        return "hello " + string(name);
    }
};

int main(int argc, char *argv[])
{
    DispatcherServlet * servlet = DispatcherServlet::instance();
    Conroller controller;
    servlet->append(HttpMethod::GET, "/hello", make_shared<HandlerExecutorChain<string, string_view>>(
        std::bind(&Conroller::hello, &controller, std::placeholders::_1)
    ));

    shared_ptr<BaseHandlerExecutorChain> handler = servlet->match(make_shared<HttpRequest>("GET", "/hello", "", ""));
    
    auto derivedHandler = dynamic_pointer_cast<HandlerExecutorChain<string, string_view>>(handler);
    if (derivedHandler) {
        auto s = (*derivedHandler)("world");
        cout << s << endl;
    } else {
        cerr << "Handler cast failed." << endl;
    }
    return 0;
}