# Build Process

## Serving from Flash or PROGMEM

### Build Flags

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

## Factory Settings
