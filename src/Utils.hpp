#pragma once

#include <time.h>
#include <Arduino.h>

class Utils
{
public:
    /**
     * @brief Convert an ISO 8601 date string to time_t
     * @param iso8601_date A String representing the date in ISO 8601 format (e.g., "2025-03-20T15:30:00.000Z")
     * @return A `time_t` value representing the date in seconds (Unix Epoch), or -1 if the conversion fails
     */
    static time_t iso8601_to_time_t(const String &iso8601_date);

    /**
     * @brief Convert a time_t value to an ISO 8601 date string
     * @param time The time_t value in seconds to convert
     * @return A String representing the date in ISO 8601 format
     */
    static String time_t_to_iso8601(time_t time_s);
};