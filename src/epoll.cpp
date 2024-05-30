#include "epoll.hpp"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <cerrno>
#include <strings.h>

Epoll::Epoll()
{
    epollfd_ = epoll_create(1);
    if (-1 == epollfd_)
    {
        printf("%s:%s:%d epoll create error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        ::close(epollfd_);
        exit(-1);
    }
}

Epoll::~Epoll()
{
    ::close(epollfd_);
}

void Epoll::update(Channel *ch)
{
    epoll_event ev;
    ev.data.ptr = ch;
    ev.events = ch->events();

    if (ch->inepoll())
    {
        if (-1 == epoll_ctl(epollfd_, EPOLL_CTL_MOD, ch->fd(), &ev))
        {
            printf("%s:%s:%d epoll event modifiy error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
            exit(-1);
        }
    }
    else
    {
        if (-1 == epoll_ctl(epollfd_, EPOLL_CTL_ADD, ch->fd(), &ev))
        {
            printf("%s:%s:%d epoll event add error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
            exit(-1);
        }
        ch->setInepoll();
    }
}

void Epoll::remove(Channel *ch)
{
    if (ch->inepoll())
    {
        if (-1 == epoll_ctl(epollfd_, EPOLL_CTL_DEL, ch->fd(), 0))
        {
            printf("%s:%s:%d epoll event remove error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
            exit(-1);
        }
    }
}

std::vector<Channel *> Epoll::loop(int timeout)
{
    std::vector<Channel *> channels;
    bzero(events_, sizeof(events_));

    int infds = epoll_wait(epollfd_, events_, MaxEvents, timeout);
    // 返回失败
    if (infds < 0)
    {
        printf("%s:%s:%d epoll event wait error: %d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        exit(-1);
    }
    // 超时。如果epool_wait()超时，表示系统很空闲，返回的channels为空。
    if (infds == 0)
    {
        printf("%s:%s:%d epoll event wait timeout.\n", __FILE__, __FUNCTION__, __LINE__);
        return channels;
    }

    // infds > 0，表示有事件发生的 fd
    for (int i = 0; i < infds; ++i)
    {
        Channel *ch = (Channel *)events_[i].data.ptr;
        ch->setRevents(events_[i].events);
        channels.push_back(ch);
    }
    return channels;
}