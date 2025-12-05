#define EMBEDDED_CLI_IMPL
#include "cli_interface.h"
#include "cli_commands.h"
#include <Logger.h>

#define TAG "CLI-INTERFACE"
// Global CLI Variables (Definition)
EmbeddedCli *cli;
CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

void cli_init()
{
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBuffer = cliBuffer;
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_BINDING_COUNT;

    cli = embeddedCliNew(config);

    if (cli == NULL)
    {
        LOG_E(TAG, "Cli was not created. Check sizes!");
        LOG_E(TAG, "CLI_BUFFER_SIZE: %d", CLI_BUFFER_SIZE);
        LOG_E(TAG, "Required size: %d", embeddedCliRequiredSize(config));

        while (1); // Halt if CLI fails to initialize
    }

    embeddedCliAddBinding(cli, {"clear",
                                "Clears the console",
                                true,
                                nullptr,
                                cli_clear});

    embeddedCliAddBinding(cli, {"version",
                                "Print version info",
                                true,
                                nullptr,
                                cli_version});

    embeddedCliAddBinding(cli, {"reboot",
                                "Reboot the device",
                                true,
                                nullptr,
                                cli_reboot});

    embeddedCliAddBinding(cli, {"hello",
                                "Print hello message",
                                true,
                                (void *)"Relay Controller",
                                cli_hello});

    embeddedCliAddBinding(cli, {"get-led-brightness",
                                "Get LED brightness",
                                true,
                                nullptr,
                                cli_get_led_brightness});

    embeddedCliAddBinding(cli, {"set-led-brightness",
                                "Set LED brightness",
                                true,
                                nullptr,
                                cli_set_led_brightness});

    embeddedCliAddBinding(cli, {"get-led-color",
                                "Get led color",
                                true,
                                nullptr,
                                cli_get_led_color});

    embeddedCliAddBinding(cli, {"set-led-color",
                                "Set led color",
                                true,
                                nullptr,
                                cli_set_led_color});

    embeddedCliAddBinding(cli, {"get-relay",
                                "Get relay status",
                                true,
                                nullptr,
                                cli_get_relay});

    embeddedCliAddBinding(cli, {"set-relay",
                                "Set relay status",
                                true,
                                nullptr,
                                cli_set_relay});

    embeddedCliAddBinding(cli, {"toggle-relay",
                                "Toggle relay status",
                                true,
                                nullptr,
                                cli_toggle_relay});

    embeddedCliAddBinding(cli, {"ctrl-buzzer",
                                "Control buzzer with a tone between 0 and 7",
                                true,
                                nullptr,
                                cli_ctrl_buzzer});
                                
    embeddedCliAddBinding(cli, {"uart-send-data",
                                "Send data over UART",
                                true,
                                nullptr,
                                cli_uart_send_data});
    // Add more commands as needed

    cli->onCommand = cli_command;
    cli->writeChar = writeChar;
    LOG_I(TAG, "Cli has started. Enter your commands.");
}

void cli_task()
{
    if (cli == NULL)
    {
        LOG_E(TAG, "Cli is not initialized!");
        return;
    }

    while (SERIAL_DBG.available() > 0)
    {
        embeddedCliReceiveChar(cli, SERIAL_DBG.read());
    }
    embeddedCliProcess(cli);
}

void writeChar(EmbeddedCli *embeddedCli, char c)
{
    SERIAL_DBG.write((uint8_t *)&c, 1);
}
