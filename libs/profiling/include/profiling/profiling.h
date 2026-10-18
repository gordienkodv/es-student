#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef PROFILING_ENABLED
#define PROFILING_ENABLED 1
#endif

#if PROFILING_ENABLED
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
#else
#define profiling_start(id) ((void)0)
#define profiling_stop(id) ((void)0)
#endif

#define TIMERS_NUMBER 5

typedef struct {
    const char* name;
    uint32_t number_starts;
    double mean_us;
    uint32_t max_us;
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);
