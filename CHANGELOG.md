# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

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
- Added Github firmware manager to pull firmware binaries from github release pages

### Changed

- Improved system status with more meaningful presentation of available data
- Improved layout on small screens
- Increased queue size for SSE and WS to 64 instead of 32

### Depreciated

### Removed

- `FT_PROJECT` feature flag removed.

### Fixed

- Fixed drawer issue introduced with DaisyUI 3.1.6

### Security

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
