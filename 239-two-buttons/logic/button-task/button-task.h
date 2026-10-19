#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef void (*button_task_callback_t)(void);

void button_task_init(uint32_t id, button_task_callback_t on_press);
void button_task_handle(void);

bool button_task_is_pressed(uint32_t id);
uint64_t button_task_get_press_count(uint32_t id);