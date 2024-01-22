# Changelog

All notable changes to this project will be documented in this file.

## [0.3.0] - WIP

> **Warning**: This update has breaking changes!

### Added

- Added postscript to platform.io build process to copy, rename and calculate MD5 checksum of \*.bin file. These files are ready for uploading to the Github Release page.
- Added more information to SystemStatus API
- Added generateToken API for security settings
- Added Multi-WiFi capability. Add more then one WiFi configuration and connect to either strongest network (default), or by priority.

### Changed

- Better route protection for user page with deep link.
- Changed build_interface.py script to check for modified files in the interface sources before re-building the interface. Saves some time on the compilation process.
- Upload firmware binary allows uploading of MD5 checksum file in advance to verify downloaded firmware package.

### Removed

- Removed support for Arduino ESP OTA.
- HttpEndpoints and Websocket Server without a securityManager are no longer possible.

### Migrate from ESPAsyncWebServer

#### Migrate `main.cpp`

Change the server and ESPSvelteKit instances to PsychicHttpServer and give the ESP32SvelteKit constructor the number of http endpoints of your project.

```
PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 115);
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
```

Add the following `build_flags` and adjust to your app, if needed:

```ini
    -D BUILD_TARGET=\"$PIOENV\"
    -D APP_NAME=\"ESP32-Sveltekit\" ; Must only contain characters from [a-zA-Z0-9-_] as this is converted into a filename
    -D APP_VERSION=\"0.3.0\" ; semver compatible version string
```

Change `-D CORE_DEBUG_LEVEL=2` to no higher than 2.

Remove the lib dependency `esphome/AsyncTCP-esphome @ ^2.0.0` and add `https://github.com/hoeken/PsychicHttp.git` instead.

#### Custom Stateful Services

Adapt the class constructor (`(PsychicHttpServer *server, ...`) to PsychicHttpServer.

Due to the loading sequence HttpEndoint and WebsocketServer both have become a `begin()` function to register their http endpoints with the server. This must be called in your stateful services' own `begin()` function:

```
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
