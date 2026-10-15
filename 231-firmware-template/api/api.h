#pragma once

#include "stdio-text-protocol.h"
#include "pico/types.h"

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