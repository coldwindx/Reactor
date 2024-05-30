#pragma once
#include <memory>
#include "loop.hpp"

class Acceptor
{
public:
    Acceptor(EventLoop *loop, const std::string &ip, const uint16_t port);
    ~Acceptor();

    void onConnect();
    void setCallback(std::function<void(std::unique_ptr<Socket>)> callback);

private:
    EventLoop *loop_;
    Socket servsock_;
    Channel acceptchanel_;
    std::function<void(std::unique_ptr<Socket>)> callback_;
};