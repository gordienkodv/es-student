#include "profiling/profiling.h"

#include "pico/stdlib.h"
#include "systime/systime.h"

typedef struct{
    const char* name;
    uint32_t total_us;
    uint32_t start_us;
    uint32_t number_starts;
    double mean_us;
    uint32_t max_us;
} profiling_stopwatch_t;

static profiling_stopwatch_t profiling_stopwatch[TIMERS_NUMBER];

void profiling_stopwatch_init(uint32_t id, const char *name)
{
    profiling_stopwatch[id].total_us = 0;
    profiling_stopwatch[id].start_us = 0;
    profiling_stopwatch[id].name = name;
    profiling_stopwatch[id].number_starts = 0;
    profiling_stopwatch[id].mean_us = 0;
    profiling_stopwatch[id].max_us = 0;
}

#if PROFILING_ENABLED
void profiling_start(uint32_t id)
{
    profiling_stopwatch[id].number_starts++;
    profiling_stopwatch[id].start_us = systime_us();
}

void profiling_stop(uint32_t id)
{
    if(profiling_stopwatch[id].number_starts)
    {
        uint32_t time_us = systime_us()-profiling_stopwatch[id].start_us;

        profiling_stopwatch[id].total_us += time_us;
        profiling_stopwatch[id].mean_us = profiling_stopwatch[id].total_us/(double)profiling_stopwatch[id].number_starts;
        if (time_us>profiling_stopwatch[id].max_us)
            profiling_stopwatch[id].max_us = time_us;
    }
}
#else
#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
#endif

bool profiling_get(uint32_t id, profiling_result_t *result)
{
    if(id<TIMERS_NUMBER && (profiling_stopwatch[id].number_starts>0))
    {
        result->name = profiling_stopwatch[id].name;
        result->number_starts = profiling_stopwatch[id].number_starts;
        result->mean_us = profiling_stopwatch[id].mean_us;
        result->max_us = profiling_stopwatch[id].max_us;

        return true;
    }
    else
        return false;
}

void profiling_reset(void)
{
    for(int id=0; id<TIMERS_NUMBER; id++)
        profiling_stopwatch_init(id, profiling_stopwatch[id].name);
}
