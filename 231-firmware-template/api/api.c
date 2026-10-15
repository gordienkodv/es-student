#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include "hardware/clocks.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "device.h"
#include "profiling.h"
#include "log.h"

#include "pico/stdlib.h"

#include <string.h>    // strcmp
#include <ctype.h>     // isdigit
#include <stdlib.h>    // strtoul
#include "pico/time.h" // time_us_64

void cmd_help(const command_t *command);
void cmd_info(const command_t *command);
void cmd_uptime(const command_t *command);
void cmd_calc_pi(const command_t *command);
void cmd_main_time_exec(const command_t *command);
void cmd_main_time_reset(const command_t *command);

const api_command_t commands[] = {
    { "help", "list of commands", cmd_help },
    { "info", "device passport", cmd_info },
    { "uptime", "time since reset", cmd_uptime },
    { "calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi },
    { "main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec },
    { "main_time_reset", "reset maximum iteration", cmd_main_time_reset },
};

const uint command_count = sizeof(commands) / sizeof(commands[0]);

void api_handle(const command_t *command)
{
    if (command->truncated)
    {
        LOG_ERR("command longer than 63 characters or 4 arguments\n");
        return;
    }



    for (uint i = 0; i < command_count; i++)
    {
        if (strcmp(command->name, commands[i].name) == 0)
        {
            if (commands[i].callback != NULL)
            {
                commands[i].callback(command);
            }

            return;
        }
    }

    LOG_ERR("unknown command: %s\n", command->name);
}

void cmd_help(const command_t *command)
{
    for (uint i = 0; i < command_count; i++)
    {
        printf("%-16s %-s\n", commands[i].name, commands[i].help);
    }
}

void cmd_info(const command_t *command)
{
    device_info();
}

void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

const uint CALC_PI_TERMS = 1000000;
double calc_pi(int n)
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
uint32_t value;

static bool parse_u32(const char *text, uint32_t *value)
{
    if (!isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}

void cmd_calc_pi(const command_t *command)
{
    if (command->argv[0] == NULL)
    {
        uint64_t start_us = time_us_64();
        pi_result = calc_pi(CALC_PI_TERMS);
        uint64_t spent_us = time_us_64() - start_us;

        printf("pi: %.8f\n", pi_result);
        printf("time: %llu ms\n", spent_us / 1000);
    }
    else if(parse_u32(command->argv[0], &value) && (command->argv[1]==NULL))
    {
        uint64_t start_us = time_us_64();
        pi_result = calc_pi(value);
        uint64_t spent_us = time_us_64() - start_us;

        printf("pi: %.9f\n", pi_result);
        printf("time: %llu ms\n", spent_us / 1000);
    }
    else
    {
        LOG_ERR("usage calc_pi [terms]\n");
    }
}










void cmd_main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

void cmd_main_time_reset(const command_t *command)
{
    profiling_reset_max();
    printf("max reset\n");
}




