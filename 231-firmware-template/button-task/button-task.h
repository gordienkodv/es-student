#pragma once

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_DOUBT,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DOUBT
} button_state_t;

void button_task_handle(void);
void button_task_init(void);
void button(void);