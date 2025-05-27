#pragma once

#include <string_view>

using std::string_view;

template <typename T>
T SetEnum(string_view name){
    throw std::runtime_error("SetEnum not implemented for this type");
}