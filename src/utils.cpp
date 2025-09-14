#include "utils.h"

#include <cstdarg> // Required for va_list, va_start, va_end
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

std::string stringFormat(const std::string &format, ...)
{
    va_list args;
    va_start(args, format);
    size_t len = std::vsnprintf(nullptr, 0, format.c_str(), args);
    va_end(args);

    if (len == static_cast<size_t>(-1))
    {
        throw std::runtime_error("Formatting error");
    }

    std::unique_ptr<char[]> buffer(new char[len + 1]);
    va_start(args, format);
    int written = std::vsnprintf(buffer.get(), len + 1, format.c_str(), args);
    va_end(args);

    if (written < 0)
    {
        throw std::runtime_error("Formatting error");
    }

    return std::string(buffer.get());
}