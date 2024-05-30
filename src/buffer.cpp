#include "buffer.hpp"
#include <string.h>

void Buffer::appendWithSep(const char *data, size_t size)
{
    if (0 == sep_)
    {
        buf_.append(data, size);
        return;
    }
    if (1 == sep_)
    {
        buf_.append((char *)&size, 4);
        buf_.append(data);
        return;
    }
    printf("Unexcept sep_ in class \"Buffer\".");
}

bool Buffer::pick(std::string &ss)
{
    if (0 == buf_.size())
        return false;
    if (0 == sep_)
    {
        ss = buf_;
        buf_.clear();
        return true;
    }
    if (1 == sep_)
    {
        int len;
        memcpy(&len, buf_.data(), 4);
        if (buf_.size() < len + 4)
            return false;

        ss = buf_.substr(4, len);
        buf_.erase(0, len + 4);
        return true;
    }
    printf("Unexcept sep_ in class \"Buffer\".");
}
