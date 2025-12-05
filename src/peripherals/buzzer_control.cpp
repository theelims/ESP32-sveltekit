#include <Arduino.h>
#include <Logger.h>
#include "buzzer_control.h"
#include "../modbus/modbus.h"

// Static method to get the singleton instance
BuzzerControl& BuzzerControl::getInstance() 
{
    static BuzzerControl instance; // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

// Private constructor
BuzzerControl::BuzzerControl() 
{
    initBuzzer(); // Initialize the buzzer during construction
}

// Initialize the buzzer (e.g., set up the GPIO pin)
// ledcSetup and ledcAttachPin are removed APIs.
// New APIs: ledcAttach used to set up the LEDC pin (merged ledcSetup and ledcAttachPin functions).
void BuzzerControl::initBuzzer()
{
    // pinMode(GPIO_PIN_Buzzer, OUTPUT); // Initialize the control GPIO of Buzzer

    // ledcAttach(GPIO_PIN_Buzzer, Frequency, Resolution); // Set a LEDC channel
    ledcAttachChannel(GPIO_PIN_Buzzer, Frequency, Resolution, PWM_Channel);   // Connect the channel to the corresponding pin
}

// Play a tone
void BuzzerControl::playTone(uint8_t tone)
{
    // Play a tone
    int noteDuration = 1000 / noteDurations[tone];
    ledcWriteTone(GPIO_PIN_Buzzer, melody[tone]);
    delay(noteDuration);
    ledcWriteTone(GPIO_PIN_Buzzer, 0); // Stop tone
    delay(50);                       // short pause between notes
}

void BuzzerControl::loop(void)
{
    std::vector<uint16_t> params = {0xFFFF};
    uint16_t quantity = params.size();
    uint16_t address = 56;

    modbus_server_get_parameters(params.data(), address, quantity);

    if (params[0] < 0)
    {
        params[0] = 0;
    }
    else if (params[0] > 8)
    {
        params[0] = 8;
    }
    else
    {
        // Do nothing here
    }
    String tempStr = String(params[0]);
    
    if (0 != params[0])
    {
        playTone(params[0]);
    }

    // LOG_I(TAG, "Get holding Register for led: " + tempStr);
}