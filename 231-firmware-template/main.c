#include "profiling.h"
#include "api.h"
#include "stdio-text-protocol.h"
#include "led-task.h"
#include "button-task.h"
#include "pi-task.h"

int main()
{
    stdio_text_protocol_init();
    led_task_init();
    button_task_init();
    profiling_init();

    while (1)
    {
        profiling_iteration();
        led_task_handle();
        button_task_handle();
        pi_task_handle();

        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }
    }
}
