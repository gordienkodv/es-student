#include "api-commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "device/device.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "systime/systime.h"

#include "button-task/button-task.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"

#include "firmware.h"

static device_info_t device_info;
static profiling_result_t profiling_result[TIMERS_NUMBER];

static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_profiling(const command_t *command);
static void cmd_profiling_reset(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_button(const command_t *command);
static void cmd_pi_start(const command_t *command);
static void cmd_pi(const command_t *command);

const api_command_t commands[] = {
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "profiling", "get profiling info", cmd_profiling },
    { "profiling_reset", "reset profiling info", cmd_profiling_reset },
    { "led_enable", "enable led", cmd_led_enable},
    { "led_disable", "disable led", cmd_led_disable},
    { "led_blink", "blink led", cmd_led_blink},
    { "led_period", "set led blink period", cmd_led_period},
    { "button", "button state", cmd_button},
    { "pi_start", "start pi calculation", cmd_pi_start},
    { "pi", "get pi value", cmd_pi},
};

const uint32_t command_count = sizeof(commands) / sizeof(commands[0]);

static bool parse_u32(const char *text, uint32_t *value)
{
    if (!isdigit((unsigned char)text[0]))
        return false;

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
        return false;

    *value = (uint32_t)number;
    return true;
}

static void cmd_info(const command_t *command)
{
    device_get_info(&device_info);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", FIRMWARE_PROJECT);
    printf("repo: %s\n", FIRMWARE_REPO);
    printf("board: %s\n", device_info.board);
    printf("serial: %s\n", device_info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", (unsigned)device_info.chip_manufacturer, (unsigned)device_info.chip_part, (unsigned)device_info.chip_revision);
    printf("pico-sdk: %s\n", device_info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", systime_us() / 1000);
}

static const uint32_t CALC_PI_TERMS = 1000000;

static void cmd_pi_start(const command_t *command)
{
    uint32_t value = 0;

    if (command->argc == 0)
    {
        if (!pi_task_start(CALC_PI_TERMS))
            printf("error: pi is busy\n");
        else
            printf("pi: started, %u terms\n", (unsigned)value);
    }
    else if( !parse_u32(command->argv[0], &value) || (command->argc!=1))
    {
        printf("error: usage pi_start [terms]\n");
    }
    else
    {
        if (!pi_task_start(value))
            printf("error: pi is busy\n");
        else
            printf("pi: started, %u terms\n", (unsigned)value);
    }
}

static void cmd_pi(const command_t *command)
{
    pi_answer_t *pi_answer = pi_task_get_result();
    switch(pi_answer->pi_state)
    {
        case PI_NEVER_STARTED:
            printf("pi: not started\n");
            break;
        case PI_RUNNING:
            printf("pi: running\n");
            break;
        case PI_FINISHED:
            printf("pi: %.9f in %u ms\n", pi_answer->pi, (unsigned)(pi_answer->total_us/1000));
            break;
    }
}

static void cmd_profiling(const command_t *command)
{
    if(PROFILING_ENABLED)
        for(int id=0; id<TIMERS_NUMBER; id++)
            if(profiling_get(id, &profiling_result[id]))
                printf("%-10s count %-10u mean %.2f us  max %u us\n", profiling_result[id].name, (unsigned)profiling_result[id].number_starts, profiling_result[id].mean_us, (unsigned)profiling_result[id].max_us);
            else{;}
    else
        printf("profiling is disabled in this build\n");
}

static void cmd_profiling_reset(const command_t *command)
{
    profiling_reset();
    printf("profiling reset\n");
}

static void print_led_state(void);

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    print_led_state();
}

static void print_led_state(void)
{
        switch (led_task_get_state()) {
    case LED_STATE_OFF:
        printf("led: off, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    case LED_STATE_ON:
        printf("led: on, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    case LED_STATE_BLINK:
        printf("led: blink, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    }
}

static void cmd_button(const command_t *command)
{
    if(button_task_is_pressed())
        printf("button: released, presses %u\n", (unsigned)button_task_get_press_count());
    else
        printf("button: pressed, presses %u\n", (unsigned)button_task_get_press_count());
}