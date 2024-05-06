# Build Process

The build process is controlled by [platformio.ini](https://github.com/theelims/ESP32-sveltekit/platformio.ini) and automates the build of the front end website with Vite as well as the binary compilation for the ESP32 firmware. Whenever PlatformIO is building a new binary it will call the python script [build_interface.py](https://github.com/theelims/ESP32-sveltekit/scripts/build_interface.py) to action. It will check the frontend files for changes. If necessary it will start the Vite build and gzip the resulting files either to the `data/` directory or embed them into a header file. In case the WWW files go into a LITTLEFS partition a file system image for the flash is created for the default build environment and upload to the ESP32.

## Changing the JS package manager

This project uses NPM as the default package manager. However, many users might have different preferences and like to use YARN or PNPM instead. Just switch the interface to one of the other package managers. The build script identify the package manager by the presence of its lock-file and start the vite build process accordingly.

## Serving from Flash or Embedding into the Binary

The front end website can be served either from the LITTLEFS partition of the flash, or embedded into the firmware binary (default). Later has the advantage that only one binary needs to be distributed easing the OTA process. Further more this is desirable if you like to preserve the settings stored in the LITTLEFS partition, or have other files there that need to survive a firmware update. To serve from the LITTLEFS partition instead please comment the following build flag out:

```ini
build_flags =
    ...
    -D EMBED_WWW
```

### Partitioning

If you choose to embed the frontend it becomes part of the firmware binary (default). As many ESP32 modules only come with 4MB built-in flash this results in the binary being too large for the reserved flash. Therefor a partition scheme with a larger section for the executable code is selected. However, this limits the LITTLEFS partition to 200kb. There are a great number of [default partition tables](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions) for Arduino-ESP32 to choose from. If you have 8MB or 16MB flash this would be your first choice. If you don't need OTA you can choose a partition scheme without OTA.

Should you want to deploy the frontend from the flash's LITTLEFS partition on a 4MB chip you need to comment out the following two lines. Otherwise the 200kb will not be large enough to host the front end code.

```ini
board_build.partitions = min_spiffs.csv
```

## Selecting Features

Many of the framework's built in features may be enabled or disabled as required at compile time. This can help save sketch space and memory if your project does not require the full suite of features. The access point and WiFi management features are "core features" and are always enabled. Feature selection may be controlled with the build flags defined in [features.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/features.ini).

Customize the settings as you see fit. A value of 0 will disable the specified feature:

```ini
  -D FT_SECURITY=1
  -D FT_MQTT=1
  -D FT_NTP=1
  -D FT_UPLOAD_FIRMWARE=1
  -D FT_DOWNLOAD_FIRMWARE=1
  -D FT_SLEEP=1
  -D FT_BATTERY=1
```

| Flag                 | Description                                                                                                                                                                                                              |
| -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| FT_SECURITY          | Controls whether the [security features](statefulservice.md#security-features) are enabled. Disabling this means you won't need to authenticate to access the device and all authentication predicates will be bypassed. |
| FT_MQTT              | Controls whether the MQTT features are enabled. Disable this if your project does not require MQTT support.                                                                                                              |
| FT_NTP               | Controls whether network time protocol synchronization features are enabled. Disable this if your project does not require accurate time.                                                                                |
| FT_UPLOAD_FIRMWARE   | Controls whether the manual upload firmware feature is enabled. Disable this if you won't be manually uploading firmware.                                                                                                |
| FT_DOWNLOAD_FIRMWARE | Controls whether the firmware download feature is enabled. Disable this if you won't firmware pulled from a server.                                                                                                      |
| FT_SLEEP             | Controls whether the deep sleep feature is enabled. Disable this if your device is not battery operated or you don't need to place it in deep sleep to save energy.                                                      |
| FT_BATTERY           | Controls whether the battery state of charge shall be reported to the clients. Disable this if your device is not battery operated.                                                                                      |

In addition custom features might be added or removed at runtime. See [Custom Features](statefulservice.md#custom-features) on how to use this in your application.

## Factory Settings

The framework has built-in factory settings which act as default values for the various configurable services where settings are not saved on the file system. These settings can be overridden using the build flags defined in [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini). All strings entered here must be escaped, especially special characters.

Customize the settings as you see fit, for example you might configure your home WiFi network as the factory default:

```ini
  -D FACTORY_WIFI_SSID=\"My\ Awesome\ WiFi\ Network\"
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

It is recommended that you change the user credentials from their defaults to better protect your device. You can do this in the user interface, or by modifying [factory_settings.ini](https://github.com/theelims/ESP32-sveltekit/blob/main/factory_settings.ini) as mentioned above.

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

## Other Build Flags

### Cross-Origin Resource Sharing

If you need to enable Cross-Origin Resource Sharing (CORS) on the ESP32 server just uncomment the following build flags:

```ini
build_flags =
...
  ; Uncomment to configure Cross-Origin Resource Sharing
  -D ENABLE_CORS
  -D CORS_ORIGIN=\"*\"
```

This will add the `Access-Control-Allow-Origin` and `Access-Control-Allow-Credentials` headers to any request made.

### ESP32 `CORE_DEBUG_LEVEL`

The ESP32 Arduino Core and many other libraries use the ESP Logging tools. To enable these debug and error messages from deep inside your libraries uncomment the following build flag.

```ini
build_flags =
...
	-D CORE_DEBUG_LEVEL=5
```

It accepts values from 5 (Verbose) to 1 (Errors) for different information depths to be logged on the serial terminal. If commented out there won't be debug messages from the core libraries. For a production build you should comment this out.

### Serve Config Files

By enabling this build flag the ESP32 will serve all config files stored on the LittleFS flash partition under `http:\\[IP]\config\[filename].json`. This can be helpful to troubleshoot problems. However, it is strongly advised to disable this for production builds.

```ini
build_flags =
...
  -D SERVE_CONFIG_FILES
```

### Serial Info

In some circumstances it might be beneficial to not print any information on the serial consol (Serial1 or USB CDC). By commenting out the following build flag ESP32-Sveltekit will not print any information on the serial console.

```ini
build_flags =
...
  -D SERIAL_INFO
```

## SSL Root Certificate Store

Some features like firmware download or the MQTT client require a SSL connection. For that the SSL Root CA certificate must be known to the ESP32. The build system contains a python script derived from Espressif ESP-IDF building a certificate store containing one or more certificates. In order to create the store you must uncomment the three lines below in `platformio.ini`.

```ini
extra_scripts =
    pre:scripts/generate_cert_bundle.py
board_build.embed_files = src/certs/x509_crt_bundle.bin
board_ssl_cert_source = adafruit
```

The script will download a public certificate store from Mozilla (`board_ssl_cert_source = mozilla`) or a repository curated by Adafruit (`board_ssl_cert_source = adafruit`), builds a binary containing all certs and embeds this into the firmware. This will add ~65kb to the firmware image. Should you only need a few known certificates you can place their `*.pem` or `*.der` files in the [ssl_certs](https://github.com/theelims/ESP32-sveltekit/blob/main/ssl_certs) folder and change `board_ssl_cert_source = folder`. Then only these certificates will be included in the store. This is especially useful, if you only need to connect to know servers and need to shave some kb off the firmware image:

!!! info

     To enable SSL the feature `FT_NTP=1` must be enabled as well.

## Vite and LittleFS 32 Character Limit

The static files for the website are build using vite. By default vite adds a unique hash value to all filenames for improved caching performance. However, LittleFS on the ESP32 is limited to filenames with 32 characters. This restricts the number of characters available for the user to name svelte files. To give a little bit more headroom a vite-plugin removes all hash values, as they offer no benefit on an ESP32. However, have the 32 character limit in mind when naming files. Excessively long names may still cause some issues when building the LittleFS binary.
