#include "profiling.h"
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include <stdio.h>

typedef struct{
    uint32_t total_us;
    uint32_t start_us;
    profiling_result_t* result;
} profiling_stopwatch_t;

static profiling_stopwatch_t profiling_stopwatch[TIMERS_NUMBER];
profiling_result_t profiling_result[TIMERS_NUMBER];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    profiling_stopwatch[id].total_us = 0;
    profiling_stopwatch[id].start_us = 0;
    profiling_stopwatch[id].result = &profiling_result[id];
    profiling_result[id].name = name;
    profiling_result[id].number_starts = 0;
    profiling_result[id].mean_us = 0;
    profiling_result[id].max_us = 0;
}

void profiling_start(uint32_t id)
{
    profiling_stopwatch[id].result->number_starts++;
    profiling_stopwatch[id].start_us = time_us_32();
}

void profiling_stop(uint32_t id)
{
    if(profiling_stopwatch[id].result->number_starts)
    {
        uint32_t time_us = time_us_32()-profiling_stopwatch[id].start_us;

        profiling_stopwatch[id].total_us += time_us;
        profiling_stopwatch[id].result->mean_us = profiling_stopwatch[id].total_us/(double)profiling_stopwatch[id].result->number_starts;
        if (time_us>profiling_stopwatch[id].result->max_us)
            profiling_stopwatch[id].result->max_us = time_us;
    }
}

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if((result->number_starts>0) && id<TIMERS_NUMBER)
        {
            printf("%-10s count %-10u mean %.2f us  max %u us\n", result->name, (unsigned)result->number_starts, result->mean_us, (unsigned)result->max_us);
            return true;
        }
    else
        return false;
}

void profiling_reset(void)
{
    for(int id=0; id<TIMERS_NUMBER; id++)
        profiling_stopwatch_init(id, profiling_stopwatch[id].result->name);
}

