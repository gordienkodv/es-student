#include "api.h"
#include "api-commands.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

static void cmd_help()
{
    for (uint32_t i = 0; i < command_count; i++)
        printf("%-16s %-s\n", commands[i].name, commands[i].help);
}

void api_handle(const command_t *command)
{
    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }

    if(strcmp(command->name, "help") == 0)
        cmd_help();
    else
        for (uint32_t i = 0; i < command_count; i++)
            if (strcmp(command->name, commands[i].name) == 0)
            {
                if (commands[i].callback != NULL)
                    commands[i].callback(command);
                return;
            }

    printf("error: unknown command: %s\n", command->name);
}