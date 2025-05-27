#pragma once

#include <memory>
#include "handler_executor_chain.h"

using std::shared_ptr;

class HandlerAdapter
{
public:
    template <typename... Args>
    auto execute(shared_ptr<BaseHandlerExecutorChain> chain, Args... args) -> decltype(std::declval<HandlerExecutorChain<Args...>>()(args...))
    {
        HandlerExecutorChain<Args...> *c = dynamic_cast<HandlerExecutorChain<Args...> *>(chain.get());
        if (c)
            return (*c)(args...);
    }
};
