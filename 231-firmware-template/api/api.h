#pragma once

#include "pico/types.h"
#include "stdio-text-protocol/stdio-text-protocol.h"

typedef void (*api_callback_t)(const command_t *command);

typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

extern const api_command_t commands[];
extern const uint command_count;

void api_handle(const command_t *command);