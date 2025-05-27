#include "web/servlet.h"

DispatcherServlet* DispatcherServlet::ins = nullptr;
std::once_flag DispatcherServlet::flag;

DispatcherServlet::DispatcherServlet()
{
    mappings[HttpMethod::GET] = std::make_shared<GetHandlerMapping>();
}

DispatcherServlet *DispatcherServlet::instance()
{
    std::call_once(flag, []()
                   { ins = new DispatcherServlet(); });
    return ins;
}