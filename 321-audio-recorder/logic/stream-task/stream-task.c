#include "stream-task/stream-task.h"

#include <stddef.h>
#include "platform/platform.h"

static stream_state_t stream_state;
static stream_task_callback_t stream_task_callback;
static uint8_t stream_bits;
static uint16_t stream_frames_number;
static uint32_t stream_samples;

void stream_task_init(stream_task_callback_t on_frame)
{
    stream_state = STREAM_STATE_IDLE;
    stream_task_callback = on_frame;
}

bool stream_task_start(uint8_t bits)
{
    if((stream_state==STREAM_STATE_RUN) || !platform_stream_start(bits))
        return false;

    stream_bits = bits;
    stream_frames_number = 0;
    stream_samples = 0;
    
    stream_state = STREAM_STATE_RUN;
    return true;
}

void stream_task_stop(void)
{
    platform_stream_stop();
    stream_state = STREAM_STATE_IDLE;
}

void stream_task_handle(void)
{
    if(stream_state!=STREAM_STATE_RUN)
        return;
    
    uint32_t samples;
    const uint8_t* stream = platform_stream_next(&samples);
    
    if(stream==NULL)
        return;

    uint32_t bytes = samples;
    if (stream_bits == 16)
        bytes *= 2;

    stream_header_t header = {
        .sync_word = STREAM_SYNC_WORD,
        .number = stream_frames_number,
        .count = samples,
    };

    stream_task_callback(&header, stream, bytes);
    platform_stream_release();

    stream_frames_number++;
    stream_samples += samples;
}

stream_state_t stream_task_get_state(void)
{
    return stream_state;
}

uint8_t stream_task_get_bits(void)
{
    return stream_bits;
}

uint32_t stream_task_get_frames(void)
{
    return stream_frames_number;
}

uint32_t stream_task_get_samples(void)
{
    return stream_samples;
}

uint32_t stream_task_get_overruns(void)
{
    return platform_stream_overruns();
}
