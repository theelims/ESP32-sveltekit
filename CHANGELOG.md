# Changelog

All notable changes to this project will be documented in this file.

## [0.2.2]

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
- getServer() returning a pointer to the AsnycWebServer instance.
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
