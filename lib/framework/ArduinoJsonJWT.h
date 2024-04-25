#ifndef ArduinoJsonJWT_H
#define ArduinoJsonJWT_H

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

#include <Arduino.h>
#include <ArduinoJson.h>
#include <libb64/cdecode.h>
#include <libb64/cencode.h>
#include <mbedtls/md.h>

class ArduinoJsonJWT
{
private:
    String _secret;

    const String JWT_HEADER = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9";
    const int JWT_HEADER_SIZE = JWT_HEADER.length();

    String sign(String &value);

    static String encode(const char *cstr, int len);
    static String decode(String value);

public:
    ArduinoJsonJWT(String secret);

    void setSecret(String secret);
    String getSecret();

    String buildJWT(JsonObject &payload);
    void parseJWT(String jwt, JsonDocument &jsonDocument);
};

#endif
