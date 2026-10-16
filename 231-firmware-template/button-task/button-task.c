#include "button-task.h"
#include "led-task.h"
#include <stdint.h>
#include "hardware/gpio.h"
#include <stdio.h>
#include "hardware/timer.h"

static const uint BUTTON_PIN = 15;
static const uint DEBOUNCE_US = 20000;
static uint64_t last_us = 0;
static uint64_t presses = 0;

static button_state_t button_state = BUTTON_STATE_RELEASED;

void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}

void button_task_handle(void)
{
    bool state = gpio_get(BUTTON_PIN);
    uint64_t now_us = time_us_64();

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
                    led_task_next_state();
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

void button(void)
{
    if((button_state==BUTTON_STATE_RELEASED) || (button_state==BUTTON_STATE_PRESS_DOUBT))
        printf("button: released, presses %u\n", (unsigned)presses);
    else
        printf("button: pressed, presses %u\n", (unsigned)presses);
}