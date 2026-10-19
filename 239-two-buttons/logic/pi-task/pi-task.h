#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PI_NEVER_STARTED,
    PI_RUNNING,
    PI_FINISHED,
} pi_state_t;

typedef struct
{
    pi_state_t pi_state;
    double pi;
    uint64_t total_us;
} pi_answer_t;

bool pi_task_start(int n);
void pi_task_handle(void);
pi_answer_t* pi_task_get_result(void);
