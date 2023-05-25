# Build Process

## Serving from Flash or PROGMEM

### Build Flags

The build process will compile the filesystem image from the `data/` directory and upload it to the device automatically.

```ini
build_flags =
    ...
    -D PROGMEM_WWW
```

### Partitioning

If you choose to serve the frontend from PROGMEM (default) it becomes part of the firmware binary. As many ESP32 modules only come with 4MB built-in flash this results in the binary being too large for the reserved flash. Therefor a partition scheme with a larger section for the executable code is selected. However, this limits the SPIFFS partition to 200kb. There are a great number of [default partition tables](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions) for Arduino-ESP32 to choose from. If you have 8MB or 16MB flash this would be your first choice. If you don't need OTA you can choose a partition scheme without OTA.

Should you want to deploy the frontend from the flash's SPIFFS partition on a 4MB chip you need to comment out the following two lines. Otherwise the 200kb will be not large enough to host the front end code.

```ini
board_build.partitions = min_spiffs.csv
```

## Selecting Features

Many of the framework's built in features may be enabled or disabled as required at compile time. This can help save sketch space and memory if your project does not require the full suite of features. The access point and WiFi management features are "core features" and are always enabled. Feature selection may be controlled with the build flags defined in [features.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/features.ini).

Customize the settings as you see fit. A value of 0 will disable the specified feature:

```ini
  -D FT_PROJECT=1
  -D FT_SECURITY=1
  -D FT_MQTT=1
  -D FT_NTP=1
  -D FT_OTA=1
  -D FT_UPLOAD_FIRMWARE=1
```

| Flag               | Description                                                                                                                                                                                                              |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| FT_PROJECT         | Controls whether the "project" section of the UI is enabled. Disable this if you don't intend to have your own screens in the UI.                                                                                        |
| FT_SECURITY        | Controls whether the [security features](statefulservice.md#security-features) are enabled. Disabling this means you won't need to authenticate to access the device and all authentication predicates will be bypassed. |
| FT_MQTT            | Controls whether the MQTT features are enabled. Disable this if your project does not require MQTT support.                                                                                                              |
| FT_NTP             | Controls whether network time protocol synchronization features are enabled. Disable this if your project does not require accurate time.                                                                                |
| FT_OTA             | Controls whether OTA update support is enabled. Disable this if you won't be using the remote update feature.                                                                                                            |
| FT_UPLOAD_FIRMWARE | Controls the whether the manual upload firmware feature is enabled. Disable this if you won't be manually uploading firmware.                                                                                            |

## Factory Settings

The framework has built-in factory settings which act as default values for the various configurable services where settings are not saved on the file system. These settings can be overridden using the build flags defined in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini).

Customize the settings as you see fit, for example you might configure your home WiFi network as the factory default:

```ini
  -D FACTORY_WIFI_SSID=\"My Awesome WiFi Network\"
  -D FACTORY_WIFI_PASSWORD=\"secret\"
  -D FACTORY_WIFI_HOSTNAME=\"awesome_light_controller\"
```

### Default access point settings

By default, the factory settings configure the device to bring up an access point on start up which can be used to configure the device:

- SSID: ESP32-Sveltekit
- Password: esp-sveltekit

### Security settings and user credentials

By default, the factory settings configure two user accounts with the following credentials:

| Username | Password |
| -------- | -------- |
| admin    | admin    |
| guest    | guest    |

It is recommended that you change the user credentials from their defaults better protect your device. You can do this in the user interface, or by modifying [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini) as mentioned above.

### Customizing the factory time zone setting

Changing factory time zone setting is a common requirement. This requires a little effort because the time zone name and POSIX format are stored as separate values for the moment. The time zone names and POSIX formats are contained in the UI code in [timezones.ts](https://github.com/theelims/ESP32-sveltekit/blob/main/interface/src/routes/connections/timezones.ts). Take the appropriate pair of values from there, for example, for Los Angeles you would use:

```ini
  -D FACTORY_NTP_TIME_ZONE_LABEL=\"America/Los_Angeles\"
  -D FACTORY_NTP_TIME_ZONE_FORMAT=\"PST8PDT,M3.2.0,M11.1.0\"
```

### Placeholder substitution

Various settings support placeholder substitution, indicated by comments in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini). This can be particularly useful where settings need to be unique, such as the Access Point SSID or MQTT client id. The following placeholders are supported:

| Placeholder  | Substituted value                                                     |
| ------------ | --------------------------------------------------------------------- |
| #{platform}  | The microcontroller platform, e.g. "esp32" or "esp32c3"               |
| #{unique_id} | A unique identifier derived from the MAC address, e.g. "0b0a859d6816" |
| #{random}    | A random number encoded as a hex string, e.g. "55722f94"              |

You may use SettingValue::format in your own code if you require the use of these placeholders. This is demonstrated in the demo project:

```cpp
  static StateUpdateResult update(JsonObject& root, LightMqttSettings& settings) {
    settings.mqttPath = root["mqtt_path"] | SettingValue::format("homeassistant/light/#{unique_id}");
    settings.name = root["name"] | SettingValue::format("light-#{unique_id}");
    settings.uniqueId = root["unique_id"] | SettingValue::format("light-#{unique_id}");
    return StateUpdateResult::CHANGED;
  }
```
