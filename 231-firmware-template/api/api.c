#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "hardware/clocks.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "device.h"
#include "profiling.h"
#include "log.h"
#include "led-task.h"

#include "pico/stdlib.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "pico/time.h"

static void cmd_help(const command_t *command);
static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_calc_pi(const command_t *command);
static void cmd_main_time_exec(const command_t *command);
static void cmd_main_time_reset(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_led_period(const command_t *command);

const api_command_t commands[] = {
    { "help", "list of commands", cmd_help },
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi },
    { "main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration", cmd_main_time_reset },
    { "led_enable", "enable led", cmd_led_enable},
    { "led_disable", "disable led", cmd_led_disable},
    { "led_blink", "blink led", cmd_led_blink},
    { "led_period", "set led blink period", cmd_led_period},
};

const uint command_count = sizeof(commands) / sizeof(commands[0]);

void api_handle(const command_t *command)
{
    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    for (uint i = 0; i < command_count; i++)
        if (strcmp(command->name, commands[i].name) == 0)
        {
            if (commands[i].callback != NULL)
                commands[i].callback(command);
            return;
        }

    printf("error: unknown command: %s\n", command->name);
}

static void cmd_help(const command_t *command)
{
    for (uint i = 0; i < command_count; i++)
        printf("%-16s %-s\n", commands[i].name, commands[i].help);
}

static void cmd_info(const command_t *command)
{
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static const uint CALC_PI_TERMS = 1000000;
static double calc_pi(int n)
{
    double sum = 0.0;
    double sign = 1.0;

    for (int k = 0; k < n; k++) {
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }

    return 4.0 * sum;
}

volatile double pi_result;

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

static void cmd_calc_pi(const command_t *command)
{
    uint32_t value;

    if (command->argv[0] == NULL)
    {
        uint64_t start_us = time_us_64();
        pi_result = calc_pi(CALC_PI_TERMS);
        uint64_t spent_us = time_us_64() - start_us;

        printf("pi: %.8f\n", pi_result);
        printf("time: %llu ms\n", spent_us / 1000);
    }
    else if(parse_u32(command->argv[0], &value) && (command->argc!=1))
    {
        uint64_t start_us = time_us_64();
        pi_result = calc_pi(value);
        uint64_t spent_us = time_us_64() - start_us;

        printf("pi: %.9f\n", pi_result);
        printf("time: %llu ms\n", spent_us / 1000);
    }
    else
    {
        printf("error: usage calc_pi [terms]\n");
    }
}

static void cmd_main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *command)
{
    profiling_reset_max();
    printf("max reset\n");
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

