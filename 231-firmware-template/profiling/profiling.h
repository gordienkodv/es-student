#pragma once

#include "pico/types.h"

#define TIMERS_NUMBER 5

typedef struct {
    const char* name;
    uint32_t number_starts;
    double mean_us;
    uint32_t max_us;
} profiling_result_t;

extern profiling_result_t profiling_result[TIMERS_NUMBER];

void profiling_stopwatch_init(uint32_t id, const char *name);
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);

