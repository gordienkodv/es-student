#include "block-task/block-task.h"

#include <stddef.h>
#include "platform/platform.h"

static block_task_callback_t block_task_callback;
static uint32_t block_count;

bool block_task_start(uint32_t count, block_task_callback_t on_block)
{
    if(platform_adc_busy() || !platform_block_start(count))
        return false;
    block_task_callback = on_block;
    block_count = count;
    return true;
}

void block_task_handle(void)
{
    if((block_task_callback==NULL) || (!platform_block_done()))
        return;

    block_task_callback_t callback_now = block_task_callback;
    block_task_callback = NULL;
    callback_now(platform_block_samples(), block_count, platform_block_time_us());
}
