#ifndef FeaturesService_h
#define FeaturesService_h

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

#include <Features.h>

#include <WiFi.h>
#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <vector>

#define FEATURES_SERVICE_PATH "/rest/features"

typedef struct
{
    String feature;
    bool enabled;
} UserFeature;

class FeaturesService
{
public:
    FeaturesService(PsychicHttpServer *server);

    void begin();

    void addFeature(String feature, bool enabled);

private:
    PsychicHttpServer *_server;
    std::vector<UserFeature> userFeatures;
};

#endif
