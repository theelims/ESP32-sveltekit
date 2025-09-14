#include "cli_commands.h"
#include "peripherals/relay_control.h"
#include "peripherals/buzzer_control.h"
#include "peripherals/digital_led_control.h"

#include <Logger.h>

#define TAG "CLI-COMMANDS"
#define CLI_INTERFACE_VERSION "0.0.1"
void cli_command(EmbeddedCli *embeddedCli, CliCommand *command)
{
    LOG_I(TAG, "Received command: %S", command->name);
    embeddedCliTokenizeArgs(command->args);
    for (int i = 1; i <= embeddedCliGetTokenCount(command->args); ++i) 
    {
        const char *arg = embeddedCliGetToken(command->args, i);
        LOG_I(TAG, "arg %d: %s", i, arg);
    }
}
void cli_clear(EmbeddedCli *cli, char *args, void *context)
{
    // SERIAL_DBG.println(F("\33[2J"));
    LOG_I(TAG, F("\33[2J"));
}
void cli_version(EmbeddedCli *cli, char *args, void *context)
{
    LOG_I(TAG, "CLI-Version: %S", CLI_INTERFACE_VERSION);
}
void cli_reboot(EmbeddedCli *cli, char *args, void *context)
{
    LOG_I(TAG, "Rebooting...");
    delay(1000);
    ESP.restart();
}
void cli_hello(EmbeddedCli *cli, char *args, void *context)
{
    LOG_I(TAG, "Hello ");
    if (args == NULL || embeddedCliGetTokenCount(args) == 0)
    {
        LOG_I(TAG, (const char *)context);
    }
    else
    {
        const char *name = embeddedCliGetToken(args, 1);
        if (name != NULL)
        {
            LOG_I(TAG, name);
        }
        else
        {
            LOG_I(TAG, "stranger");
        }
    }
}

void cli_get_led_brightness(EmbeddedCli *cli, char *args, void *context)
{
    // Set the LED color using the DigitalLedControl class
    DigitalLedControl& ledControl = DigitalLedControl::getInstance();
    uint8_t color = ledControl.getBrightness();

    LOG_I(TAG, "Get LED brightness: %d", color);
}

void cli_set_led_brightness(EmbeddedCli *cli, char *args, void *context)
{
    if ((args == NULL) || (embeddedCliGetTokenCount(args) < 1))
    {
        LOG_I(TAG, "Usage: set-led-brightness [arg1]");
        return;
    }

    const char *arg1 = embeddedCliGetToken(args, 1);
    uint8_t brightness = constrain(atoi(arg1), 0, 255); // Constrain brightness value

    // Set the LED brightness using the DigitalLedControl class
    DigitalLedControl& ledControl = DigitalLedControl::getInstance();
    ledControl.setBrightness(brightness); 

    LOG_I(TAG, "Set LED brightness: %d", brightness);
}   

void cli_get_led_color(EmbeddedCli *cli, char *args, void *context)
{
    // Set the LED color using the DigitalLedControl class
    DigitalLedControl& ledControl = DigitalLedControl::getInstance();
    uint32_t color = ledControl.getColor();
    uint8_t rVal = (color >> 16) & 0xFF;
    uint8_t gVal = (color >> 8) & 0xFF;
    uint8_t bVal = color & 0xFF;

    LOG_I(TAG, "Get LED color: %d, R-%d, G-%d, B-%d", color, rVal, gVal, bVal);
}

void cli_set_led_color(EmbeddedCli *cli, char *args, void *context)
{
    if ((args == NULL) || (embeddedCliGetTokenCount(args) < 3))
    {
        LOG_I(TAG, "Usage: set-led [arg1] [arg2] [arg3]");
        return;
    }

    const char *arg1 = embeddedCliGetToken(args, 1);
    const char *arg2 = embeddedCliGetToken(args, 2);
    const char *arg3 = embeddedCliGetToken(args, 3);

    uint8_t rVal = constrain(atoi(arg1), 0, 255); // Constrain red value
    uint8_t gVal = constrain(atoi(arg2), 0, 255); // Constrain green value
    uint8_t bVal = constrain(atoi(arg3), 0, 255); // Constrain blue value

    // Set the LED color using the DigitalLedControl class
    DigitalLedControl& ledControl = DigitalLedControl::getInstance();
    uint8_t brightness = ledControl.getBrightness();
    ledControl.setBrightness(brightness);
    ledControl.setColor(rVal, gVal, bVal);

    LOG_I(TAG, "Set LED with RGB values: R-%d, G-%d, B-%d", rVal, gVal, bVal);
}

void cli_get_relay(EmbeddedCli *cli, char *args, void *context)
{
    RelayControl& relayCtrl = RelayControl::getInstance();

    if (args == NULL)
    {
        std::string status = relayCtrl.printStatus();
        LOG_I(TAG, "All Relay Status: %s", status.c_str());
        return;
    }
    
    const char *arg1 = embeddedCliGetToken(args, 1);
    const uint8_t channel = atoi(arg1);
    // Validate channel number
    if (channel < 0 || channel >= RELAY_CHANNEL_COUNT) 
    {    
        LOG_W(TAG, "channel number. Use a number between 0 and 5.");
        return;
    }

    bool status = relayCtrl.getChannel(channel);

    LOG_I(TAG, "Get relay - %d; status %s", channel, status ? "ON" : "OFF");
}
void cli_set_relay(EmbeddedCli *cli, char *args, void *context)
{
    if ((args == NULL) || (embeddedCliGetTokenCount(args) < 1))
    {
        LOG_I(TAG, "Usage: set-relay [channel] [on/off] or set-relay all [on/off]");
        return;
    }

    const char *arg1 = embeddedCliGetToken(args, 1);

    RelayControl& relayCtrl = RelayControl::getInstance();

    // Handle "all" command
    if (strcmp(arg1, "all") == 0)
    {
        if (embeddedCliGetTokenCount(args) < 2)
        {
            LOG_I(TAG, "Usage: set-relay all [on/off]");
            return;
        }

        const char *arg2 = embeddedCliGetToken(args, 2);
        if (strcmp(arg2, "on") == 0 || strcmp(arg2, "1") == 0)
        {
            relayCtrl.allOn();
            LOG_I(TAG, "All relays turned ON.");
        }
        else if (strcmp(arg2, "off") == 0 || strcmp(arg2, "0") == 0)
        {
            relayCtrl.allOff();
            LOG_I(TAG, "All relays turned OFF.");
        }
        else
        {
            LOG_W(TAG, "state. Use 'on' or 'off'.");
        }
        return;
    }

    // Handle specific channel command
    if (embeddedCliGetTokenCount(args) < 2)
    {
        LOG_I(TAG, "Usage: set-relay [channel] [on/off]");
        return;
    }

    const uint8_t channel = atoi(arg1);
    if (channel < 0 || channel >= RELAY_CHANNEL_COUNT)
    {
        LOG_W(TAG, "channel number. Use a number between 0 and 5.");
        return;
    }

    const char *arg2 = embeddedCliGetToken(args, 2);
    bool newState = false;

    if (strcmp(arg2, "on") == 0 || strcmp(arg2, "1") == 0)
    {
        newState = true;
    }
    else if (strcmp(arg2, "off") == 0 || strcmp(arg2, "0") == 0)
    {
        newState = false;
    }
    else
    {
        LOG_W(TAG, "state. Use 'on' or 'off'.");
        return;
    }

    relayCtrl.setChannel(channel, newState);

    LOG_I(TAG, "Set relay - %d; status %s", channel, newState ? "ON" : "OFF");
}

void cli_toggle_relay(EmbeddedCli *cli, char *args, void *context)
{
    if ((args == NULL) || (embeddedCliGetTokenCount(args) < 1))
    {
        LOG_I(TAG, "Usage: toggle-relay [arg1]");
        return;
    }

    const char *arg1 = embeddedCliGetToken(args, 1);
    const uint8_t channel = atoi(arg1);
    // Validate channel number
    if (channel < 0 || channel >= RELAY_CHANNEL_COUNT) 
    {    
        LOG_W(TAG, "channel number. Use a number between 0 and 5.");
        return;
    }

    RelayControl& relayCtrl = RelayControl::getInstance();
    relayCtrl.toggleChannel(channel);
    bool status = relayCtrl.getChannel(channel);

    LOG_I(TAG, "Toggle relay - %d; new status %s", channel, status ? "ON" : "OFF");
}

void cli_ctrl_buzzer(EmbeddedCli *cli, char *args, void *context)
{
    const char *arg1 = embeddedCliGetToken(args, 1);
    if (arg1 == NULL) 
    {
        LOG_I(TAG, "Usage: ctrl-buzzer [arg1]");
        return;
    }

    const uint8_t tone = atoi(arg1);
    // Validate tone
    if (tone < 0 || tone >= 8) 
    {    
        LOG_W(TAG, "channel number. Use a number between 0 and 7.");
        return;
    }

    BuzzerControl& buzzer = BuzzerControl::getInstance();
    buzzer.playTone(tone); 

    LOG_I(TAG, "Control buzzer with tone %d", tone);
}

void cli_uart_send_data(EmbeddedCli *cli, char *args, void *context)
{
    if ((args == NULL) || (embeddedCliGetTokenCount(args) < 1))
    {
        LOG_I(TAG, "Usage: uart-send-data [data]");
        return;
    }

    // Extract the data to send
    const char *data = embeddedCliGetToken(args, 1);

    // Placeholder for UARTCommunication class

    LOG_I(TAG, "UART data sent: %s", data);
}