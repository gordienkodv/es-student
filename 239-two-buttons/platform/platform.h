#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BUTTON1,
    BUTTON2,
    BUTTON_COUNT,
} platform_button_t;

void platform_init(void);
void platform_led_set(bool on);
bool platform_button_read(uint32_t id);