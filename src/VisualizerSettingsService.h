#pragma once

#include <FSPersistence.h>
#include <HttpEndpoint.h>
#include <JsonUtils.h>
#include <SecurityManager.h>
#include <PsychicHttp.h>
#include <ESP32SvelteKit.h>
#include <Utils.hpp>

#define PACKET_VISUALIZER_SETTINGS_FILE "/config/packet-visualizer.json"
#define PACKET_VISUALIZER_SETTINGS_SERVICE_PATH "/rest/packet-visualizer"

#define PACKET_VISUALIZER_SETTINGS_STR_SHOW_DETAILS "showDetails"
#define PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_DETAILS true
#define PACKET_VISUALIZER_SETTINGS_STR_SHOW_METADATA "showMetadata"
#define PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_METADATA true

class VisualizerSettings
{

public:
    bool showDetails = PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_DETAILS;
    bool showMetadata = PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_METADATA;

    static void read(VisualizerSettings &vizSettings, JsonObject &root)
    {
        root[PACKET_VISUALIZER_SETTINGS_STR_SHOW_DETAILS] = vizSettings.showDetails;
        root[PACKET_VISUALIZER_SETTINGS_STR_SHOW_METADATA] = vizSettings.showMetadata;

        ESP_LOGV(VisualizerSettings::TAG, "Packet visualizer settings read.");
    }

    static StateUpdateResult update(JsonObject &root, VisualizerSettings &vizSettings)
    {
        vizSettings.showDetails = root[PACKET_VISUALIZER_SETTINGS_STR_SHOW_DETAILS] | PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_DETAILS;
        vizSettings.showMetadata = root[PACKET_VISUALIZER_SETTINGS_STR_SHOW_METADATA] | PACKET_VISUALIZER_SETTINGS_DEFAULT_SHOW_METADATA;

        ESP_LOGV(VisualizerSettings::TAG, "Packet visualizer settings updated.");

        return StateUpdateResult::UNCHANGED;
    }

protected:
    static constexpr const char *TAG = "VisualizerSettings";
};

class VisualizerSettingsService : public StatefulService<VisualizerSettings>
{
public:
    VisualizerSettingsService(ESP32SvelteKit *sveltekit);

    void begin();

private:
    HttpEndpoint<VisualizerSettings> _httpEndpoint;
    FSPersistence<VisualizerSettings> _fsPersistence;
};
