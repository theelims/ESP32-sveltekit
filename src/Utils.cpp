#include <Utils.hpp>

time_t Utils::iso8601_to_time_t(const String& iso8601_date) {
    struct tm tm = {0};
    int millis = 0;     // Ignore milliseconds
    if (sscanf(iso8601_date.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%3dZ", 
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday, 
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &millis) != 7) {
        return -1;
    }
    tm.tm_year -= 1900; // Adjust year
    tm.tm_mon -= 1;     // Adjust month (0-based)
    return mktime(&tm);

    // TODO: Time Zone correction?
}

String Utils::time_t_to_iso8601(time_t time_s) {
    struct tm *tm = gmtime(&time_s);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S.000Z", tm);
    return String(buf);

    // TODO: Time Zone correction?
}