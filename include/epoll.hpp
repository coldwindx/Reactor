#pragma once
#include <sys/epoll.h>
#include <vector>
#include "channel.hpp"

class Channel;

class Epoll
{
public:
    Epoll();
    ~Epoll();

    void update(Channel * ch);
    void remove(Channel * ch);
    std::vector<Channel*> loop(int timeout = -1);
private:
    static const int MaxEvents = 1024;
    int epollfd_ = -1;
    epoll_event events_[MaxEvents];
};