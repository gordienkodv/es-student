#include "pi-task.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <stdint.h>

#define PI_TERMS_PER_CALL 100

typedef enum {
    PI_NEVER_STARTED,
    PI_RUNNING,
    PI_FINISHED,
} pi_state_t;

static pi_state_t pi_state = PI_NEVER_STARTED;
static uint64_t total_us = 0;
static uint32_t pi_term_need = 1000000;
static uint32_t pi_terms_now = 0;
static double pi;
static int pi_sign = 1;

bool pi_task_start(int n)
{
    if(pi_state==PI_RUNNING)
        return false;
    else
    {
        pi_state = PI_RUNNING;
        total_us = time_us_64();
        pi_terms_now = 0;
        pi_term_need = n;
        pi_sign = 1;
        pi = 0.0;
        printf("pi: started, %u terms\n", n);
    }
    return true;
}

void pi_task_handle(void)
{
    if ((pi_state==PI_NEVER_STARTED) || (pi_state==PI_FINISHED))
        return;

    uint32_t end = pi_terms_now + PI_TERMS_PER_CALL;

    if (pi_term_need-pi_terms_now < PI_TERMS_PER_CALL)
        end = pi_term_need;

    for (; pi_terms_now < end; pi_terms_now++)
    {
        pi += pi_sign / (2.0*pi_terms_now+1.0);
        pi_sign = -pi_sign;
    }

    if (end == pi_term_need)
    {
        pi *= 4.0;
        pi_state = PI_FINISHED;
        total_us = time_us_64()-total_us;
    }
}

void pi_task_get_result(void)
{
    switch(pi_state)
    {
        case PI_NEVER_STARTED:
            printf("pi: not started\n");
            break;
        case PI_RUNNING:
            printf("pi: running\n");
            break;
        case PI_FINISHED:
            printf("pi: %f.9 in %u ms\n", pi, (unsigned)(total_us/1000));
            break;
    }
}

