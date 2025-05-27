#pragma once

#include <functional>

using std::function;

class BaseHandlerExecutorChain
{
public:
    virtual ~BaseHandlerExecutorChain() = default;
};

template <typename R, typename... Args>
class HandlerExecutorChain : public BaseHandlerExecutorChain
{
    function<R(Args...)> f_;

public:
    HandlerExecutorChain() = delete;
    HandlerExecutorChain(function<R(Args...)> f) : f_(f) {}
    R operator()(Args... args) { return f_(args...); }
};