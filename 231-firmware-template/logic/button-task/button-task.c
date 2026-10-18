#include "button-task/button-task.h"

#include "systime/systime.h"
#include "platform/platform.h"

static const uint64_t DEBOUNCE_US = 20000;
static uint64_t last_us;
static uint64_t presses;
button_task_callback_t button_task;

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_DOUBT,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DOUBT
} button_state_t;

static button_state_t button_state;

void button_task_init(button_task_callback_t on_press)
{
    last_us = 0;
    presses = 0;
    button_state = BUTTON_STATE_RELEASED;
    button_task = on_press;
}

void button_task_handle(void)
{
    bool state = platform_button_read();
    uint64_t now_us = systime_us();

    switch (button_state)
    {
        case BUTTON_STATE_RELEASED:
            if(!state)
            {
                button_state = BUTTON_STATE_PRESS_DOUBT;
                last_us = now_us;
            }
            break;
        case BUTTON_STATE_PRESS_DOUBT:
            if(!state)
                if (now_us-last_us > DEBOUNCE_US)
                {
                    button_state = BUTTON_STATE_PRESSED;
                    presses++;
                    button_task();
                }else;
            else
                button_state = BUTTON_STATE_RELEASED;
            break;
        case BUTTON_STATE_PRESSED:
            if(state)
            {
                button_state = BUTTON_STATE_RELEASE_DOUBT;
                last_us = now_us;
            }
            break;
        case BUTTON_STATE_RELEASE_DOUBT:
            if(state)
                if (now_us-last_us > DEBOUNCE_US)
                    button_state = BUTTON_STATE_RELEASED;
                else;
            else
                button_state = BUTTON_STATE_PRESSED;
            break;
    }
}

bool button_task_is_pressed(void)
{
    if((button_state==BUTTON_STATE_RELEASED) || (button_state==BUTTON_STATE_PRESS_DOUBT))
        return true;
    else
        return false;
}
uint64_t button_task_get_press_count(void)
{
    return presses;
}
