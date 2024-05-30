#pragma once
#include <string>

class Buffer
{
public:
    Buffer(uint16_t sep = 1) : sep_(sep) {}
    ~Buffer() = default;

    void append(const char *data, size_t size) { buf_.append(data, size); }
    void appendWithSep(const char *data, size_t size);
    void erase(size_t pos, size_t nn) { buf_.erase(pos, nn); }
    size_t size() { return buf_.size(); }
    const char *data() { return buf_.c_str(); }
    void clear() { buf_.clear(); }
    bool pick(std::string & ss);
private:
    std::string buf_;
    /*
     @brief The separator of the packet.
     @brief if 0 then indicates no separator.
     @brief if 1 then indicates a 4-byte header.
     @brief if 2 then indicates that \\r\\n\\r\\n separator is used.
    */
    const uint16_t sep_;
};