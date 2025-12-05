#ifndef DIGITAL_LED_CONTROL_H
#define DIGITAL_LED_CONTROL_H

#include <Arduino.h>
#include <FastLED.h>
#include "board_def.h"

#define NUM_LEDS 1  // Number of WS2812B LEDs

class DigitalLedControl 
{
public:
    // Get the singleton instance
    static DigitalLedControl& getInstance();

    // Delete copy constructor and assignment operator to prevent copying
    DigitalLedControl(const DigitalLedControl&) = delete;
    DigitalLedControl& operator=(const DigitalLedControl&) = delete;

    void loop(void);
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void setColor(uint32_t color);
    void setBrightness(uint8_t brightness);

    uint32_t getColor() const;
    uint8_t getBrightness() const;

    void turnOff();

private:
    // Private constructor for Singleton
    DigitalLedControl();

    // Private destructor
    ~DigitalLedControl() = default;

    void initLeds();
    void show();

private:
    CRGB leds[NUM_LEDS];  // LED array
    uint8_t brightness;   // LED brightness
    uint32_t color;       // LED color stored as a 32-bit integer
    const char *TAG = "LED-Ctrl";
};

#endif