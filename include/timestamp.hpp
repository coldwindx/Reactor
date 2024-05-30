#pragma once
#include <ctime>
#include <string>

class Timestamp
{
public:
    Timestamp();
    Timestamp(int64_t sec);

    static Timestamp now();

    time_t toint() const;
    std::string tostring() const;

private:
    time_t sec_;
};