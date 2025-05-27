#pragma once

#include <map>
#include <memory>
#include <mutex>
#include "http/httpmethod.h"
#include "http/httprequest.h"
#include "mapping.h"

using std::map, std::shared_ptr;

/**
 * @brief 前端控制器，负责将请求根据路由分配给对应的处理器，单例模式
 */
class DispatcherServlet
{
    static std::once_flag flag;
    static DispatcherServlet *ins;

    map<HttpMethod, shared_ptr<HandlerMapping>> mappings;

    DispatcherServlet();

public:
    ~DispatcherServlet() = delete;

    static DispatcherServlet *instance();

    shared_ptr<BaseHandlerExecutorChain> match(shared_ptr<HttpRequest> request)
    {
        HttpMethod method = SetEnum<HttpMethod>(request->getMethod());
        if (method == HttpMethod::GET){
            return mappings[HttpMethod::GET]->match(request->getUrl());
        }
    }

    void append(HttpMethod method, string_view url, shared_ptr<BaseHandlerExecutorChain> handler)
    {
        mappings[method]->append(url, handler);
    }
};