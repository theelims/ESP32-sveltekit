#include <Arduino.h>
#include <Logger.h>
#include <Relay.h>
#include "relay_control.h"
#include "../modbus/modbus.h"


RelayControl& RelayControl::getInstance() 
{
    static RelayControl instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

// Private constructor
RelayControl::RelayControl() 
{
    initRelays(); // Initialize relays during construction
}

void RelayControl::loop(void)
{
    std::vector<uint16_t> params = {0, 0, 0, 0, 0, 0, 0, 0};
    uint16_t quantity = params.size();
    String tempStr = "";
    uint16_t address = 32;

    modbus_server_get_parameters(params.data(), address, quantity);
    for (uint16_t i = 0; i < quantity; i++)
    {   
        if (i < (quantity - 1))
        {
            tempStr += String(params[i]) + ",";
        }
        else
        {
            tempStr += String(params[i]);
        }

        if (0 == params[i])
        {
            setChannel(i, false);
        }
        else if (1 == params[i])
        {
            setChannel(i, true);
        }
        else if (2 == params[i])
        {
            toggleChannel(i);
        }
        else
        {
            // Do nothing here
        }
    }
    // LOG_I(TAG, "Get holding Register for relay: " + tempStr);
}
void RelayControl::setChannel(uint8_t channel, bool state) 
{
    if (channel < RELAY_CHANNEL_COUNT) 
    {
        relays[channel].setState(state);
    }
}

bool RelayControl::getChannel(uint8_t channel) 
{
    if (channel < RELAY_CHANNEL_COUNT) 
    {
        return relays[channel].getState();
    }
    return false; // Return false if the channel is out of range
}

void RelayControl::toggleChannel(uint8_t channel) 
{
    if (channel < RELAY_CHANNEL_COUNT) 
    {
        bool state = !getChannel(channel);
        relays[channel].setState(state);
    }
}

void RelayControl::allOff() 
{
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; i++) 
    {
        relays[i].setState(false);
    }
}

void RelayControl::allOn() 
{
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; i++) 
    {
        relays[i].setState(true);
    }
}

std::string RelayControl::printStatus() 
{
    std::string json = "{";
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; i++) 
    {
        json += "\"Relay" + std::to_string(i + 1) + "\": ";
        json += relays[i].getState() ? "true" : "false";
        if (i < RELAY_CHANNEL_COUNT - 1) 
        {
            json += ", ";
        }
    }
    json += "}";
    return json;
}

void RelayControl::initRelays() 
{
    for (uint8_t i = 0; i < RELAY_CHANNEL_COUNT; i++) 
    {
        relays[i] = Relay(RELAY_PINS[i]);
    }
}

