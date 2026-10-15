#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-text-protocol.h"
#include "log.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint line_length = 0;
static bool truncated = false;
command_t command;

void stdio_text_protocol_init(void)
{
    stdio_init_all();
}

const command_t *stdio_text_protocol_handle(void)
{
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
        return NULL;

    if (symbol == '\r' || symbol == '\n')
    {
        putchar('\n');
        line[line_length] = '\0';

        if (line_length > 0)
        {
            LOG_DBG("got %s\n", line);

            command.name = &line[0];
            command.argc = 0;
            symbol = 0;
            for (uint i = 0; i < STDIO_TEXT_PROTOCOL_MAX_ARGS; i++)
                command.argv[i] = NULL;

            for (uint i = 0; i < line_length; i++)
            {
                if (line[i] == ' ')
                {
                    line[i] = '\0';
                    symbol = 1;
                }
                else if (symbol == 1)
                {
                    if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS)
                    {
                        command.argv[command.argc] = &line[i];
                        command.argc++;
                        symbol = 0;
                    }
                    else
                        truncated = true;
                }
            }
            command.truncated = truncated;
        }
        else
            return NULL;

        line_length = 0;
        truncated = false;

        return &command;
    }

    if (symbol == 0x08 || symbol == 0x7f)
    {
        if (line_length > 0)
        {
            line_length--;
            putchar('\b');
            putchar(' ');
            putchar('\b');
        }
        return NULL;
    }

    if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE)
    {
        line[line_length] = (char)symbol;
        line_length = line_length + 1;
        putchar(symbol);
    }
    else
        truncated = true;

    return NULL;
}
