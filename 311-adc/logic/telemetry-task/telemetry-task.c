#include "telemetry-task.h"

#include "systime/systime.h"
#include "platform/platform.h"

static telemetry_record_t telemetry_record;
static telemetry_state_t telemetry_state;
static telemetry_task_callback_t telemetry_task;
static uint32_t telemetry_period_ms = 100;
static uint64_t last_telemetry;
static bool telemetry_force;

void telemetry_task_init(telemetry_task_callback_t on_record)
{
    telemetry_state = TELEMETRY_STATE_IDLE;
    telemetry_task = on_record;
}

void telemetry_task_handle(void)
{
    switch (telemetry_state) {
    case TELEMETRY_STATE_IDLE:
        break;
    case TELEMETRY_STATE_RUN:
        if (systime_period_elapsed(&last_telemetry, telemetry_period_ms*1000) || telemetry_force)
            {
                telemetry_force = false;
                telemetry_record.time_us = systime_us();
                telemetry_record.voltage_v = platform_voltage_read();
                telemetry_record.temp_c = platform_temp_read();
                telemetry_task(&telemetry_record);                
            }
        break;
    }
}

bool telemetry_task_start(uint32_t period_ms)
{
    if(period_ms < 10)
        return false;
    else
    {
        telemetry_period_ms = period_ms;
        telemetry_state = TELEMETRY_STATE_RUN;
        telemetry_force = true;
        return true;
    }
}

void telemetry_task_stop(void)
{
    telemetry_state = TELEMETRY_STATE_IDLE;
}

telemetry_state_t telemetry_task_get_state(void)
{
    return telemetry_state;
}

uint32_t telemetry_task_get_period_ms(void)
{
    return telemetry_period_ms;
}