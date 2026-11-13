#include "pi-task/pi-task.h"

#include "systime/systime.h"

#define PI_TERMS_PER_CALL 100

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
        total_us = systime_us();
        pi_terms_now = 0;
        pi_term_need = n;
        pi_sign = 1;
        pi = 0.0;
        return true;
    }
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
        total_us = systime_us()-total_us;
    }
}

pi_answer_t* pi_task_get_result(void)
{
    static pi_answer_t pi_answer;

    pi_answer.pi_state = pi_state;
    pi_answer.pi = pi;
    pi_answer.total_us = total_us;

    return &pi_answer;
}

