/**
 *   Copyright (C) 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the Attribution-ShareAlike 4.0 International license.
 *   See the LICENSE file for details.
 *
 **/

#ifndef STATUS_MONITOR_H
#define STATUS_MONITOR_H

#include <ESP32SvelteKit.h>
#include <Arduino.h>
#include <OSSMReferenceBoard.h>
#include <FastLED.h>

#define smTAG "StatusMonitor"

#ifndef STATUS_UPDATE_INTERVAL
#define STATUS_UPDATE_INTERVAL 500
#endif

class StatusMonitor
{
public:
    StatusMonitor(ESP32SvelteKit *esp32sveltekit) : _esp32sveltekit(esp32sveltekit) {}

    void begin()
    {
        FastLED.addLeds<WS2812, NEOPIXEL_PIN, GRB>(led, 1);
    }

    void loop()
    {
        if (millis() - _lastUpdate > STATUS_UPDATE_INTERVAL)
        {
            FastLED.setBrightness(64);

            switch (_esp32sveltekit->getConnectionStatus())
            {
            case ConnectionStatus::OFFLINE:
                led[0] = CRGB::White;
                break;
            case ConnectionStatus::AP:
                led[0] = CRGB::Blue;
                break;
            case ConnectionStatus::AP_CONNECTED:
                led[0] = CRGB::Cyan;
                break;
            case ConnectionStatus::STA:
                led[0] = CRGB::Red;
                break;
            case ConnectionStatus::STA_CONNECTED:
                led[0] = CRGB::Magenta;
                break;
            case ConnectionStatus::STA_MQTT:
                led[0] = CRGB::Green;
                break;
            default:
                led[0] = CRGB::Black;
                break;
            }
            FastLED.show();
            _lastUpdate = millis();
        }
    }

protected:
    ESP32SvelteKit *_esp32sveltekit;
    unsigned long _lastUpdate = 0;
    CRGB led[1];
};

#endif // STATUS_MONITOR_H