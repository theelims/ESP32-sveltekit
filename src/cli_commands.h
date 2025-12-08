#ifndef CLI_COMMANDS_H
#define CLI_COMMANDS_H

#include <Arduino.h>
#include "embedded_cli.h"

void cli_command(EmbeddedCli *embeddedCli, CliCommand *command);
void cli_clear(EmbeddedCli *cli, char *args, void *context);
void cli_version(EmbeddedCli *cli, char *args, void *context);
void cli_reboot(EmbeddedCli *cli, char *args, void *context);
void cli_hello(EmbeddedCli *cli, char *args, void *context);

void cli_get_led_brightness(EmbeddedCli *cli, char *args, void *context);
void cli_set_led_brightness(EmbeddedCli *cli, char *args, void *context);

void cli_get_led_color(EmbeddedCli *cli, char *args, void *context);
void cli_set_led_color(EmbeddedCli *cli, char *args, void *context);

void cli_get_relay(EmbeddedCli *cli, char *args, void *context);
void cli_set_relay(EmbeddedCli *cli, char *args, void *context);
void cli_toggle_relay(EmbeddedCli *cli, char *args, void *context);

void cli_ctrl_buzzer(EmbeddedCli *cli, char *args, void *context);

void cli_uart_send_data(EmbeddedCli *cli, char *args, void *context);

#endif