#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include <Arduino.h>
#include "embedded_cli.h"

// Configuration Constants
#define CLI_BUFFER_SIZE (1024 * 8)
#define CLI_RX_BUFFER_SIZE (1024)
#define CLI_CMD_BUFFER_SIZE (1024)
#define CLI_HISTORY_SIZE (64)
#define CLI_BINDING_COUNT (128)

// Global CLI Variables
extern EmbeddedCli *cli;
extern CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

// Function Prototypes
void writeChar(EmbeddedCli *embeddedCli, char c);

void cli_init();
void cli_task();

#endif