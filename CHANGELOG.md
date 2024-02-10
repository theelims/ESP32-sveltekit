# Changelog

All notable changes to this project will be documented in this file.

## [WIP] - Work in Progress

### Added

- Added build flag `-D SERIAL_INFO` to platformio.ini to enable / disable all `Serial.print()` statements. On some boards with native USB those Serial prints have been reported to block anc make the server unresponsive.

### Changed

- more generic board definition in platformio.ini (#20)
- refactored MqttPubSub.h into a single class to improve readability

### Fixed

- Duplicate method in FeatureService (#18)
- Fixed compile error with FLAG `-D SERVE_CONFIG_FILES`

## [0.3.0] - 2023-02-05

> [!CAUTION]
> This update has breaking changes!

This is a major change getting rid of all ESPAsyncTCP and ESPAsyncWebserver dependencies. Despite their popularity they are plagued with countless bugs, since years unmaintained, not SSL capable and simply not suitable for a production build. Although several attempts exist to fix the most pressing bugs even these libraries lead to frequent crashes. This new version replaces them with ESP-IDF based components. [PsychicHttp](https://github.com/hoeken/PsychicHttp) and [PsychicMqttClient](https://github.com/theelims/PsychicMqttClient) both wrap the ESP-IDF components in a familiar wrapper for easy porting of the code base. However, this will break existing code and will require some effort on your codebase. In return the stability is improved greatly and the RAM usage more friendly. Now e.g. running Bluetooth in parallel becomes possible.

### Added

- Added postscript to platform.io build process to copy, rename and calculate MD5 checksum of \*.bin file. These files are ready for uploading to the Github Release page.
- Added more information to SystemStatus API
- Added generateToken API for security settings
- Added Multi-WiFi capability. Add up to five WiFi configurations and connect to either strongest network (default), or by priority.
- Added InfoDialog as a simpler version of the ConfirmDialog for a simple notification modal.
- Added Adafruit certificate repository as the default choice for the X509 certificate bundle.

### Changed

- Better route protection for user page with deep link.
- Changed build_interface.py script to check for modified files in the interface sources before re-building the interface. Saves some time on the compilation process.
- Upload firmware binary allows uploading of MD5 checksum file in advance to verify downloaded firmware package.
- GithubFirmwareManager checks against PIO build_target in filename to support Github OTA for binaries build for various targets. You should rename your old release \*.bin files on the Github release pages for backward compatibility.
- Changed MQTT Client to an ESP-IDF backed one which supports SSL/TLS X509 root CA bundles and transport over WS.
- Changed the `PROGMEM_WWW` flag to `EMBED_WWW` as there is technically speaking no PROGMEM on ESP32's.
- Updated dependencies to the latest version. Except SvelteKit.

### Fixed

- Fixed reactivity of System Status page.

### Removed

- Removed support for Arduino ESP OTA.
- HttpEndpoints and Websocket Server without a securityManager are no longer possible.

### Migrate from ESPAsyncWebServer to PsychicHttp

#### Migrate `main.cpp`

Change the server and ESPSvelteKit instances to PsychicHttpServer and give the ESP32SvelteKit constructor the number of http endpoints of your project.

```
PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 120);
```

Remove `server.begin();` in `void setup()`. This is handled by ESP32SvelteKit now.

#### Migrate `platformio.ini`

Remove the following `build_flags`:

```ini
    ; Increase queue size of SSE and WS
    -D SSE_MAX_QUEUED_MESSAGES=64
    -D WS_MAX_QUEUED_MESSAGES=64
    -D CONFIG_ASYNC_TCP_RUNNING_CORE=0
    -D NO_GLOBAL_ARDUINOOTA
    -D PROGMEM_WWW
```

Add the following `build_flags` and adjust to your app, if needed:

```ini
    -D BUILD_TARGET=\"$PIOENV\"
    -D APP_NAME=\"ESP32-Sveltekit\" ; Must only contain characters from [a-zA-Z0-9-_] as this is converted into a filename
    -D APP_VERSION=\"0.3.0\" ; semver compatible version string
    -D EMBED_WWW
```

Remove the lib dependency `esphome/AsyncTCP-esphome @ ^2.0.0` and add `https://github.com/theelims/PsychicMqttClient.git`

Consider adjusting `board_ssl_cert_source = adafruit`, so that the new MQTT client has universal SSL/TLS support with a wide range of CA root certificates.

#### Migrate `factory_settings.ini`

The new MQTT client has slightly renamed factory settings:

```ini
  ; MQTT settings
  -D FACTORY_MQTT_ENABLED=false
  -D FACTORY_MQTT_URI=\"mqtts://mqtt.eclipseprojects.io:8883\"
  -D FACTORY_MQTT_USERNAME=\"\" ; supports placeholders
  -D FACTORY_MQTT_PASSWORD=\"\"
  -D FACTORY_MQTT_CLIENT_ID=\"#{platform}-#{unique_id}\" ; supports placeholders
  -D FACTORY_MQTT_KEEP_ALIVE=120
  -D FACTORY_MQTT_CLEAN_SESSION=true
```

Max Topic Length is no longer needed.

#### Custom Stateful Services

Adapt the class constructor (`(PsychicHttpServer *server, ...`) to PsychicHttpServer.

Due to the loading sequence HttpEndoint and WebsocketServer both have gotten a `begin()` function to register their http endpoints with the server. This must be called in your stateful services' own `begin()` function:

```cpp
void LightStateService::begin()
{
    _httpEndpoint.begin();
    _webSocketServer.begin();
    _state.ledOn = DEFAULT_LED_STATE;
    onConfigUpdated();
}
```

## [0.2.2] - 2023-10-08

### Added

- Status reports reset-reason & uptime.
- AnalyticsService to draw graphs about heap usage and other time dependent values
- Added ping to WebSocket Server
- Use telemetry store with RSSI messages to gauge health of connection. Automatic reconnect for SSE and WS.
- Added user supplied features to FeatureService
- Compiler flag to let it serve the config JSONs for debug purposes
- Hard fork of ESPAsyncWebserver as it is unmaintained to fix bugs and add features

### Changed

- Changed JSON format for websocket server and removed "payload" property. JSON is the same as for MQTT or HTTP now.
- Changed features.ini to default `FT_SLEEP=0`
- Updated dependencies to latest version.

## [0.2.1] - 2023-09-11

### Fixed

- Fixed the boot loop issue for Arduino 6.4.0

## [0.2.0] - 2023-08-03

### Added

- Introduced CHANGELOG.md
- Added core temperature to the system status API
- Added mDNS / Bonjour / zeroConf for better discoverability in local network
- Added recovery mode which forces AP to spin up regardless from its settings
- Added push notification service to show notification toasts on all clients
- Added SSE to update RSSI in status bar on client
- Added firmware version to System Status API
- Added sleep service to send ESP32 into deep sleep. Wake-up with button using EXT1
- Added battery service to show battery state of charge in the status bar. Uses SSE.
- Added download firmware manager to pull firmware binaries e.g. from github release pages
- modified generate_cert_bundle.py from Espressif included into the build process to automatically create SSL Root CA Bundle

### Changed

- Improved system status with more meaningful presentation of available data
- Improved layout on small screens
- Increased queue size for SSE and WS to 64 instead of 32
- ESP32-SvelteKit loop()-function is its own task now
- ArduinoOTA handle runs in own task now
- AsyncTCP tasks run on Core 0 to move all networking related stuff to Core 0 and free up Core 1 for business logic
- Compiler flag on which core ESP32-sveltekit tasks should run
- Renamed WebSocketRxTx.h to WebSocketServer.h to create a distinction between WS Client and WS Server interfaces
- Made code of LightStateExample slightly more verbose
- getServer() returning a pointer to the AsyncWebServer instance.
- Updated frontend dependencies and packages to newest version.

### Depreciated

- ArduinoOTA feature is set to depreciate. It is unstable with mDNS causing some reset loops until it finally settles.

### Removed

- `FT_PROJECT` feature flag removed.

## [0.1.0] - 2023-05-18

This is the initial release of ESP32-sveltekit. With this it is feature complete to [rjwats/esp8266-react](https://github.com/rjwats/esp8266-react), where it forked from.

### Added

- Added copyright notes

### Changed

- Renaming into ESP32-sveltekit
- Small changes to reflect the slightly different file structure of sveltekit
- Build process for sveltekit

### Removed

- Dropping support for ESP8266
