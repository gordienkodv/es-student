#include "button-task/button-task.h"

#include "systime/systime.h"
#include "platform/platform.h"

static const uint64_t DEBOUNCE_US = 20000;

typedef enum {
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESS_DOUBT,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASE_DOUBT
} button_state_t;

typedef struct
{
    uint64_t last_us;
    uint64_t presses;
    button_task_callback_t button_task_callback;
    button_state_t button_state;
} button_task_t;

static button_task_t button_task[BUTTON_COUNT];

void button_task_init(uint32_t id, button_task_callback_t on_press)
{
    button_task[id].last_us = 0;
    button_task[id].presses = 0;
    button_task[id].button_state = BUTTON_STATE_RELEASED;
    button_task[id].button_task_callback = on_press;
}

void button_task_handle(void)
{
    for(uint32_t id=0; id<BUTTON_COUNT; id++)
    {
        bool state = platform_button_read(id);
        uint64_t now_us = systime_us();

        switch (button_task[id].button_state)
        {
            case BUTTON_STATE_RELEASED:
                if(!state)
                {
                    button_task[id].button_state = BUTTON_STATE_PRESS_DOUBT;
                    button_task[id].last_us = now_us;
                }
                break;
            case BUTTON_STATE_PRESS_DOUBT:
                if(!state)
                    if (now_us-button_task[id].last_us > DEBOUNCE_US)
                    {
                        button_task[id].button_state = BUTTON_STATE_PRESSED;
                        button_task[id].presses++;
                        button_task[id].button_task_callback();
                    }else;
                else
                    button_task[id].button_state = BUTTON_STATE_RELEASED;
                break;
            case BUTTON_STATE_PRESSED:
                if(state)
                {
                    button_task[id].button_state = BUTTON_STATE_RELEASE_DOUBT;
                    button_task[id].last_us = now_us;
                }
                break;
            case BUTTON_STATE_RELEASE_DOUBT:
                if(state)
                    if (now_us-button_task[id].last_us > DEBOUNCE_US)
                        button_task[id].button_state = BUTTON_STATE_RELEASED;
                    else;
                else
                    button_task[id].button_state = BUTTON_STATE_PRESSED;
                break;
        }
    }   
}

bool button_task_is_pressed(uint32_t id)
{
    if((button_task[id].button_state==BUTTON_STATE_RELEASED) || (button_task[id].button_state==BUTTON_STATE_PRESS_DOUBT))
        return true;
    else
        return false;
}
uint64_t button_task_get_press_count(uint32_t id)
{
    return button_task[id].presses;
}
