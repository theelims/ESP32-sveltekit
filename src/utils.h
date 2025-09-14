#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <string>
#include <cstdarg> // Required for va_list, va_start, va_end

std::string stringFormat(const std::string& format, ...);

#endif // UTILS_H