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
 *   Copyright (C) 2023 - 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <Features.h>

#include <WiFi.h>
#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <EventSocket.h>
#include <vector>

#define FEATURES_SERVICE_PATH "/rest/features"
#define FEATURES_SERVICE_EVENT "features"

typedef struct
{
    String feature;
    bool enabled;
} UserFeature;

class FeaturesService
{
public:
    FeaturesService(PsychicHttpServer *server, EventSocket *socket);

    void begin();

    void addFeature(String feature, bool enabled);

private:
    PsychicHttpServer *_server;
    EventSocket *_socket;
    std::vector<UserFeature> userFeatures;

    void createJSON(JsonObject &root);
};

#endif
