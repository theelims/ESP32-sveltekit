#ifndef RELAY_CONTROL_H
#define RELAY_CONTROL_H

#include <Arduino.h>
#include <string>
#include <Relay.h>

#include "board_def.h"

#define RELAY_CHANNEL_COUNT 6

const uint8_t RELAY_PINS[RELAY_CHANNEL_COUNT] = 
{
    GPIO_PIN_CH1, 
    GPIO_PIN_CH2, 
    GPIO_PIN_CH3,
    GPIO_PIN_CH4,
    GPIO_PIN_CH5,
    GPIO_PIN_CH6
};

class RelayControl 
{
public:
    // Get the singleton instance
    static RelayControl& getInstance();

    // Delete copy constructor and assignment operator to prevent copying
    RelayControl(const RelayControl&) = delete;
    RelayControl& operator=(const RelayControl&) = delete;

    void loop(void);
    inline uint8_t getChannelCount() const { return RELAY_CHANNEL_COUNT; }
    void setChannel(uint8_t channel, bool state);
    bool getChannel(uint8_t channel);
    void toggleChannel(uint8_t channel);
    void allOff();
    void allOn();
    std::string printStatus();

private:
    // Private constructor for Singleton
    RelayControl();

    // Private destructor
    ~RelayControl() = default;

    void initRelays();

    Relay relays[RELAY_CHANNEL_COUNT];
    const char *TAG = "Relay-Ctrl";
};

#endif