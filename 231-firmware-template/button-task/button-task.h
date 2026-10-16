#pragma once
#include <stdint.h>
#include <stdbool.h>

void button_task_handle(void);
void button_task_init(void);
void button(void);

bool button_task_is_pressed(void);
uint64_t button_task_get_press_count(void);