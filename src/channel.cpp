#include "channel.hpp"
#include <stdio.h>
#include <unistd.h>
#include <address.h>
#include <socket.hpp>
#include <strings.h>
#include <string.h>
#include "connection.hpp"

void Channel::useEt()
{
    events_ |= EPOLLET;
}

void Channel::enableReading()
{
    events_ |= EPOLLIN;
    loop_->updatechannel(this);
}

void Channel::disableReading()
{
    events_ &= ~EPOLLIN;
    loop_->updatechannel(this);
}

void Channel::enableWriting()
{
    events_ |= EPOLLOUT;
    loop_->updatechannel(this);
}

void Channel::disableWriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updatechannel(this);
}

void Channel::disableAll()
{
    events_ = 0;
    loop_->updatechannel(this);
}

void Channel::removeFromEventLoop()
{
    disableAll();
    loop_->removechannel(this);
}

void Channel::handle()
{
    // case-1: [不重要]对方已关闭，有些系统检测不到，可以使用EPOLLIN，recv()返回0
    if (revents_ & EPOLLRDHUP)
    {
        closecallback_();
        return;
    }
    // case-2: 接受缓冲区中有数据可以读 (EPOLLIN: 普通数据 | EPOLLPRI: 带外数据)
    if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        readcallback_();
        return;
    }
    // case-3: 有数据需要写
    if (revents_ & EPOLLOUT)
    {
        writecallback_();
        return;
    }
    // case-4: 其他事件，视为错误
    errorcallback_();
}
