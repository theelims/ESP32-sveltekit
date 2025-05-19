#ifndef IPUtils_h
#define IPUtils_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <IPAddress.h>

const IPAddress IP_NOT_SET = IPAddress(INADDR_NONE);

class IPUtils
{
public:
    static bool isSet(const IPAddress &ip)
    {
        return ip != IP_NOT_SET;
    }
    static bool isNotSet(const IPAddress &ip)
    {
        return ip == IP_NOT_SET;
    }
};

#endif // end IPUtils_h
