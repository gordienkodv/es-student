#include "platform.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

static const uint64_t BUTTON_PIN1 = 15;
static const uint64_t BUTTON_PIN2 = 14;

void platform_init(void)
{
    stdio_init_all();

    gpio_init(BUTTON_PIN1);
    gpio_set_dir(BUTTON_PIN1, GPIO_IN);
    gpio_pull_up(BUTTON_PIN1);

    gpio_init(BUTTON_PIN2);
    gpio_set_dir(BUTTON_PIN2, GPIO_IN);
    gpio_pull_up(BUTTON_PIN2);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void platform_led_set(bool on)
{
    gpio_put(PICO_DEFAULT_LED_PIN, on);
}

bool platform_button_read(uint32_t id)
{
    switch(id)
    {
        case BUTTON1:
            return gpio_get(BUTTON_PIN1);
        case BUTTON2:
            return gpio_get(BUTTON_PIN2);
    }

    return true;
}