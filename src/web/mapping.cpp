#include <string>
#include "web/mapping.h"

using std::string;

shared_ptr<BaseHandlerExecutorChain> HandlerMapping::match(string_view path)
{
    auto it = routers.find(path);
    if (it != routers.end())
        return it->second;
    throw std::runtime_error("The routing processor cannot exist: " + string(path));
}

void HandlerMapping::append(string_view path, shared_ptr<BaseHandlerExecutorChain> handler)
{
    auto it = routers.find(path);
    if (it != routers.end())
        throw std::runtime_error("The routing processor already exists: " + string(path));
    routers[path] = handler;
}
