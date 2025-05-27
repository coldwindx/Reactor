#pragma once
#include <string.h>
#include <string>
#include <string_view>
#include <stdexcept>
#include "enumutils.h"

using std::string, std::string_view;

enum class HttpMethod
{
#define HTTP_METHOD_ENUM(x) x,
#include "httpmethod.def"
#undef HTTP_METHOD_ENUM
};

static string_view GetEnumName(HttpMethod method)
{
#define HTTP_METHOD_ENUM(x) \
    case HttpMethod::x:                 \
        return #x;
    switch (method)
    {
#include "httpmethod.def"
    }

#undef HTTP_METHOD_ENUM
}

template <>
inline HttpMethod SetEnum<HttpMethod>(string_view name)
{
#define HTTP_METHOD_ENUM(x)                             \
    if (0 == strncasecmp(name.data(), #x, name.size())) \
        return HttpMethod::x;
#include "httpmethod.def"
    throw std::runtime_error("Illegal http method: " + string(name) + "\n");
#undef HTTP_METHOD_ENUM
}