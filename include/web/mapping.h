#pragma once

#include <string_view>
#include <map>
#include <memory>
#include "handler_executor_chain.h"
using std::string_view, std::shared_ptr, std::map;

class HandlerMapping
{
protected:
    map<string_view, shared_ptr<BaseHandlerExecutorChain>> routers;

public:
    virtual ~HandlerMapping() = default;
    shared_ptr<BaseHandlerExecutorChain> match(string_view path);
    void append(string_view path, shared_ptr<BaseHandlerExecutorChain> handler);
};

class GetHandlerMapping : public HandlerMapping
{
};

class PostHandlerMapping : public HandlerMapping
{
};