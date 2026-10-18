#include "led-task/led-task.h"

#include "systime/systime.h"
#include "platform/platform.h"

static led_state_t led_state;
static uint64_t half_period_us;
static uint64_t last_toggle_us;
static bool blink_state;

void led_task_init(void)
{
    led_state = LED_STATE_OFF;
    half_period_us = 500000;
    last_toggle_us = 0;
    blink_state = false;
    led_task_set_state(LED_STATE_OFF);
}

void led_task_handle(void)
{
    switch (led_state) {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, half_period_us))
            {
                platform_led_set(!blink_state);
                blink_state = !blink_state;
            }
        break;
    }
}

void led_task_set_state(led_state_t state)
{
    led_state = state;

    switch (led_state)
    {
    case LED_STATE_OFF:
        platform_led_set(false);
        blink_state = false;
        break;
    case LED_STATE_ON:
        platform_led_set(true);
        blink_state = true;
        break;
    case LED_STATE_BLINK:
        last_toggle_us = 0;
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0) {
        return false;
    }

    half_period_us = (uint64_t)period_ms * 1000 / 2;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return half_period_us/500;
}

void led_task_next_state(void)
{
    switch (led_state)
    {
    case LED_STATE_OFF:
        led_task_set_state(LED_STATE_ON);
        break;
    case LED_STATE_ON:
        led_task_set_state(LED_STATE_BLINK);
        break;
    case LED_STATE_BLINK:
        led_task_set_state(LED_STATE_OFF);
        break;
    }
}
