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

#include <FeaturesService.h>

FeaturesService::FeaturesService(PsychicHttpServer *server, EventSocket *eventsocket) : _server(server), _socket(eventsocket)
{
}

void FeaturesService::begin()
{
    _server->on(FEATURES_SERVICE_PATH, HTTP_GET, [&](PsychicRequest *request)
                {
                    PsychicJsonResponse response = PsychicJsonResponse(request, false);
                    JsonObject root = response.getRoot();
                    createJSON(root);
                    return response.send(); });

    ESP_LOGV(SVK_TAG, "Registered GET endpoint: %s", FEATURES_SERVICE_PATH);

    _socket->registerEvent(FEATURES_SERVICE_EVENT);

    _socket->onSubscribe(FEATURES_SERVICE_EVENT, [&](const String &originId)
                         {
                             ESP_LOGV(SVK_TAG, "Sending features to %s", originId.c_str());
                             JsonDocument doc;
                             JsonObject root = doc.as<JsonObject>();
                             createJSON(root);
                             _socket->emitEvent(FEATURES_SERVICE_EVENT, root); });
}

void FeaturesService::addFeature(String feature, bool enabled)
{
    UserFeature newFeature;
    newFeature.feature = feature;
    newFeature.enabled = enabled;

    userFeatures.push_back(newFeature);

    JsonDocument doc;
    JsonObject root = doc.as<JsonObject>();
    createJSON(root);
    _socket->emitEvent(FEATURES_SERVICE_EVENT, root);
}

void FeaturesService::createJSON(JsonObject &root)
{

#if FT_ENABLED(FT_SECURITY)
    root["security"] = true;
#else
    root["security"] = false;
#endif
#if FT_ENABLED(FT_MQTT)
    root["mqtt"] = true;
#else
    root["mqtt"] = false;
#endif
#if FT_ENABLED(FT_NTP)
    root["ntp"] = true;
#else
    root["ntp"] = false;
#endif
#if FT_ENABLED(FT_UPLOAD_FIRMWARE)
    root["upload_firmware"] = true;
#else
    root["upload_firmware"] = false;
#endif
#if FT_ENABLED(FT_DOWNLOAD_FIRMWARE)
    root["download_firmware"] = true;
#else
    root["download_firmware"] = false;
#endif
#if FT_ENABLED(FT_SLEEP)
    root["sleep"] = true;
#else
    root["sleep"] = false;
#endif
#if FT_ENABLED(FT_BATTERY)
    root["battery"] = true;
#else
    root["battery"] = false;
#endif
#if FT_ENABLED(FT_ANALYTICS)
    root["analytics"] = true;
#else
    root["analytics"] = false;
#endif

#if FT_ENABLED(EVENT_USE_JSON)
    root["event_use_json"] = true;
#else
    root["event_use_json"] = false;
#endif

    root["firmware_version"] = APP_VERSION;
    root["firmware_name"] = APP_NAME;
    root["firmware_built_target"] = BUILD_TARGET;

    // Iterate over user features
    for (auto &element : userFeatures)
    {
        root[element.feature.c_str()] = element.enabled;
    }
}