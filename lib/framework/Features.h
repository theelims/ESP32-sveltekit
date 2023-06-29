#ifndef Features_h
#define Features_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#define FT_ENABLED(feature) feature

// security feature on by default
#ifndef FT_SECURITY
#define FT_SECURITY 1
#endif

// mqtt feature on by default
#ifndef FT_MQTT
#define FT_MQTT 1
#endif

// ntp feature on by default
#ifndef FT_NTP
#define FT_NTP 1
#endif

// OTA feature on by default
#ifndef FT_OTA
#define FT_OTA 1
#endif

// upload firmware feature off by default
#ifndef FT_UPLOAD_FIRMWARE
#define FT_UPLOAD_FIRMWARE 0
#endif

// ESP32 sleep states off by default
#ifndef FT_SLEEP
#define FT_SLEEP 0
#endif

#endif
