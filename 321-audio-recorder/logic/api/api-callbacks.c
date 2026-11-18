#include "api-commands.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "device/device.h"
#include "profiling/profiling.h"
#include "stdio-text-protocol/stdio-text-protocol.h"
#include "systime/systime.h"

#include "button-task/button-task.h"
#include "led-task/led-task.h"
#include "pi-task/pi-task.h"
#include "block-task/block-task.h"
#include "stream-task/stream-task.h"
#include "platform/platform.h"
#include "telemetry-task/telemetry-task.h"

#include "firmware.h"

#define TELEMETRY_DEFAULT_PERIOD_MS 100
#define STREAM_BLINK_PERIOD_MS 250

static device_info_t device_info;
static profiling_result_t profiling_result[TIMERS_NUMBER];

static void cmd_info(const command_t *command);
static void cmd_uptime(const command_t *command);
static void cmd_profiling(const command_t *command);
static void cmd_profiling_reset(const command_t *command);
static void cmd_led_enable(const command_t *command);
static void cmd_led_disable(const command_t *command);
static void cmd_led_blink(const command_t *command);
static void cmd_led_period(const command_t *command);
static void cmd_button(const command_t *command);
static void cmd_pi_start(const command_t *command);
static void cmd_pi(const command_t *command);
static void cmd_get_voltage(const command_t *command);
static void cmd_get_temp(const command_t *command);
static void cmd_tm_start(const command_t *command);
static void cmd_tm_stop(const command_t *command);
static void cmd_charge_on(const command_t *command);
static void cmd_charge_off(const command_t *command);
static void cmd_timer_blink_start(const command_t *command);
static void cmd_timer_blink_stop(const command_t *command);
static void cmd_cap_start(const command_t *command);
static void cmd_cap_dump(const command_t *command);
static void cmd_dma_cap(const command_t *command);
static void cmd_stream_start(const command_t *command);
static void cmd_stream_stop(const command_t *command);
static void cmd_generate_tone(const command_t *command);
static void cmd_hang(const command_t *command);
static void cmd_wdt_enable(const command_t *command);
static void cmd_reset_reason(const command_t *command);
static void cmd_mic_bias(const command_t *command);
static void cmd_mic_level(const command_t *command);

const api_command_t commands[] = {
    { "info", "device passport", cmd_info},
    { "uptime", "time since reset", cmd_uptime},
    { "profiling", "get profiling info", cmd_profiling},
    { "profiling_reset", "reset profiling info", cmd_profiling_reset},
    { "led_enable", "enable led", cmd_led_enable},
    { "led_disable", "disable led", cmd_led_disable},
    { "led_blink", "blink led", cmd_led_blink},
    { "led_period", "set led blink period", cmd_led_period},
    { "button", "button state", cmd_button},
    { "pi_start", "start pi calculation", cmd_pi_start},
    { "pi", "get pi value", cmd_pi},
    { "get_voltage", "voltage on GP26, ADC input 0", cmd_get_voltage},
    { "get_temp", "chip temperature, degrees Celsius", cmd_get_temp},
    { "tm_start", "start telemetry", cmd_tm_start},
    { "tm_stop", "stop telemetry", cmd_tm_stop},
    { "charge_on", "enable rc-charge", cmd_charge_on},
    { "charge_off", "disable rc-charge", cmd_charge_off},
    { "timer_blink_start", "blink led by timer: timer_blink_start <period_ms>", cmd_timer_blink_start},
    { "timer_blink_stop", "stop timer blink, number of toggles", cmd_timer_blink_stop},
    { "cap_start", "start timer", cmd_cap_start},
    { "cap_dump", "dump timer", cmd_cap_dump},
    { "dma_cap", "start caption", cmd_dma_cap},
    { "stream_start", "start stream", cmd_stream_start},
    { "stream_stop", "stop stream", cmd_stream_stop},
    { "generate_tone", "generate tone", cmd_generate_tone},
    { "hang", "stop superloop", cmd_hang},
    { "wdt_enable", "enable wdt", cmd_wdt_enable},
    { "reset_reason", "reset stop reason in wdt", cmd_reset_reason},
    { "mic_bias", "mic bias", cmd_mic_bias},
    { "mic_level", "mic level", cmd_mic_level},
};

const uint32_t command_count = sizeof(commands) / sizeof(commands[0]);

static bool parse_u32(const char *text, uint32_t *value)
{
    if (!isdigit((unsigned char)text[0]))
        return false;

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
        return false;

    *value = (uint32_t)number;
    return true;
}

static void cmd_info(const command_t *command)
{
    device_get_info(&device_info);
    printf("name: %s\n", FIRMWARE_NAME);
    printf("version: %s\n", FIRMWARE_VERSION);
    printf("project: %s\n", FIRMWARE_PROJECT);
    printf("repo: %s\n", FIRMWARE_REPO);
    printf("board: %s\n", device_info.board);
    printf("serial: %s\n", device_info.serial);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", (unsigned)device_info.chip_manufacturer, (unsigned)device_info.chip_part, (unsigned)device_info.chip_revision);
    printf("pico-sdk: %s\n", device_info.sdk_version);
}

static void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", systime_us() / 1000);
}

static const uint32_t CALC_PI_TERMS = 1000000;

static void cmd_pi_start(const command_t *command)
{
    uint32_t value = 0;

    if (command->argc == 0)
    {
        if (!pi_task_start(CALC_PI_TERMS))
            printf("error: pi is busy\n");
        else
            printf("pi: started, %u terms\n", (unsigned)CALC_PI_TERMS);
    }
    else if( !parse_u32(command->argv[0], &value) || (command->argc!=1))
    {
        printf("error: usage pi_start [terms]\n");
    }
    else
    {
        if(value == 0)
            {
                printf("error: terms must be greater than 0\n");
                return;
            }
        if (!pi_task_start(value))
            printf("error: pi is busy\n");
        else
            printf("pi: started, %u terms\n", (unsigned)value);
    }
}

static void cmd_pi(const command_t *command)
{
    pi_answer_t *pi_answer = pi_task_get_result();
    switch(pi_answer->pi_state)
    {
        case PI_NEVER_STARTED:
            printf("pi: not started\n");
            break;
        case PI_RUNNING:
            printf("pi: running\n");
            break;
        case PI_FINISHED:
            printf("pi: %.9f in %u ms\n", pi_answer->pi, (unsigned)(pi_answer->total_us/1000));
            break;
    }
}

static void cmd_profiling(const command_t *command)
{
    if(PROFILING_ENABLED)
        for(int id=0; id<TIMERS_NUMBER; id++)
            if(profiling_get(id, &profiling_result[id]))
                printf("%-10s count %-10u mean %.2f us  max %u us\n", profiling_result[id].name, (unsigned)profiling_result[id].number_starts, profiling_result[id].mean_us, (unsigned)profiling_result[id].max_us);
            else{;}
    else
        printf("profiling is disabled in this build\n");
}

static void cmd_profiling_reset(const command_t *command)
{
    profiling_reset();
    printf("profiling reset\n");
}

static void print_led_state(void);

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    print_led_state();
}

static void print_led_state(void)
{
        switch (led_task_get_state()) {
    case LED_STATE_OFF:
        printf("led: off, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    case LED_STATE_ON:
        printf("led: on, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    case LED_STATE_BLINK:
        printf("led: blink, period %u ms\n", (unsigned)led_task_get_period_ms());
        break;
    }
}

static void cmd_button(const command_t *command)
{
    if(button_task_is_pressed())
        printf("button: released, presses %u\n", (unsigned)button_task_get_press_count());
    else
        printf("button: pressed, presses %u\n", (unsigned)button_task_get_press_count());
}

static void cmd_get_voltage(const command_t *command)
{
    if(platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }

    printf("voltage: %.3f V\n", platform_voltage_read());
}

static void cmd_get_temp(const command_t *command)
{
    if(platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }

    printf("temp: %.1f C\n", platform_temp_read());
}


static void cmd_tm_start(const command_t *command)
{
    uint32_t value = TELEMETRY_DEFAULT_PERIOD_MS;

    if (command->argc!=0)
    {
        if(!parse_u32(command->argv[0], &value) || (command->argc!=1))
        {
            printf("error: usage tm_start [<period_ms>]\n");
            return;
        }
    }

    if(platform_adc_busy())
    {
        printf("error: adc busy\n");
        return;
    }
    
    if(!telemetry_task_start(value))
        printf("error: period_ms must be at least 10\n");
    else
    {
        printf("telemetry: started, period %u ms\n", (unsigned)telemetry_task_get_period_ms());
    }
}

static void cmd_tm_stop(const command_t *command)
{
    telemetry_task_stop();
    printf("telemetry: stopped\n");
}

static void cmd_charge_on(const command_t *command)
{
    platform_charge_set(true);
    printf("charge: on\n");
}

static void cmd_charge_off(const command_t *command)
{
    platform_charge_set(false);
    printf("charge: off\n");
}

static void cmd_timer_blink_start(const command_t *command)
{
    uint32_t value;

    if (command->argc != 1 || !parse_u32(command->argv[0], &value))
    {
        printf("error: usage timer_blink_start <period_ms>\n");
        return;
    }

    if (!platform_blink_start(value))
    {
        printf("error: period_ms must be 10..60000\n");
        return;
    }

    led_task_set_state(LED_STATE_OFF);
    printf("timer_blink: started, period %u ms\n", (unsigned)value);
}

static void cmd_timer_blink_stop(const command_t *command)
{
    printf("timer_blink: stopped, %u toggles\n", (unsigned)platform_blink_stop());
}

static void cmd_cap_start(const command_t *command)
{
    uint32_t value1;
    uint32_t value2;

    if ((command->argc != 2) || !parse_u32(command->argv[0], &value1) || !parse_u32(command->argv[1], &value2))
    {
        printf("error: usage cap_start <N> <period_us>\n");
        return;
    }

    if((value1==0) || (value1 > PLATFORM_CAPTURE_MAX) || (value2 < 10))
    {
        printf("error: N must be 1..2000, period_us at least 10\n");
        return;
    }

    if(platform_adc_busy() || telemetry_task_get_state()==TELEMETRY_STATE_RUN)
    {
        printf("error: adc busy\n");
        return;
    }

    platform_capture_start(value1, value2);
    printf("capture: started, %u samples, period %u us\n", (unsigned)value1, (unsigned)value2);
}

static void cmd_cap_dump(const command_t *command)
{
    if(!platform_capture_done())
    {
        printf("error: no capture\n");
        return;
    }
    
    platform_capture_t capture;
    platform_capture_get(&capture);

    printf("capture: %u samples, period %u us, interval %u..%u us, handler %.1f %%", (unsigned)capture.count, (unsigned)capture.period_us, (unsigned)capture.min_interval_us, (unsigned)capture.max_interval_us, capture.handler_percent);

    for (uint32_t i = 0; i < capture.count; i++)
    {
        if((i%16)==0)
            printf("\ncap:");
        printf(" %u", (unsigned)capture.samples[i]);
    }

    printf("\n");
}

static void print_dma_cap(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    uint16_t min_code = samples[0];
    uint16_t max_code = samples[0];
    float mean_code = samples[0];

    for(uint32_t i = 1; i < count; i++)
    {
        if (samples[i] < min_code)
            min_code = samples[i];
        if (samples[i] > max_code)
            max_code = samples[i];
        mean_code += samples[i];
    }

    mean_code /= count;
    uint64_t expect_time_us = (uint64_t)count*1000000/PLATFORM_SAMPLE_RATE_HZ;

    printf("dma_cap: %u samples in %u us, expected %u us, min %u max %u mean %.1f\n", (unsigned)count, (unsigned)time_us, (unsigned)expect_time_us, (unsigned)min_code, (unsigned)max_code, mean_code);
}

static void cmd_dma_cap(const command_t *command)
{
    uint32_t value;

    if ((command->argc != 1) || !parse_u32(command->argv[0], &value))
    {
        printf("error: usage dma_cap <N>\n");
        return;
    }

    if ((value == 0) || (value>PLATFORM_BLOCK_MAX))
    {
        printf("error: N must be 1..8192\n");
        return;
    }

    if(platform_adc_busy() || !block_task_start(value, &print_dma_cap))
    {
        printf("error: adc busy\n");
        return;
    }
}

static void cmd_stream_start(const command_t *command)
{
    uint32_t value;


    if(command->argc == 0)
        value = 16;
    else if ((command->argc != 1) || !parse_u32(command->argv[0], &value) || !((value==8) || (value==16)))
    {
        printf("error: usage stream_start [8|16]\n");
        return;
    }

    if(platform_adc_busy() || !stream_task_start(value))
    {
        printf("error: adc busy\n");
        return;
    }

    printf("stream: started, 44100 Hz, %u bit\n", (unsigned)stream_task_get_bits());
    led_task_set_state(LED_STATE_OFF);
    platform_blink_start(STREAM_BLINK_PERIOD_MS);
}

static void cmd_stream_stop(const command_t *command)
{
    stream_task_stop();
    platform_blink_stop();
    printf("stream: stopped, %u frames, %u samples, %u overruns\n", 
        (unsigned)stream_task_get_frames(), (unsigned)stream_task_get_samples(), (unsigned)stream_task_get_overruns());
}

static void cmd_generate_tone(const command_t *command)
{
    uint32_t value;

    if ((command->argc != 1) || !parse_u32(command->argv[0], &value))
    {
        printf("error: usage generate_tone <Hz>\n");
        return;
    }

    if (!platform_tone_set(value))
    {
        printf("error: Hz must be 0 or 20..20000\n");
        return;
    }

    if(value==0)
        printf("tone: off\n");
    else
        printf("tone: %u Hz\n", (unsigned)value);
}

static void cmd_hang(const command_t *command)
{
    printf("hang: superloop stopped\n");

    while (1)
    {
    }
}

static void cmd_wdt_enable(const command_t *command)
{
    uint32_t value;

    if ((command->argc != 1) || !parse_u32(command->argv[0], &value))
    {
        printf("error: usage wdt_enable <timeout_ms>\n");
        return;
    }

    if (!platform_watchdog_enable(value))
    {
        printf("error: timeout_ms must be 1..8388\n");
        return;
    }

    printf("wdt: enabled, timeout %u ms\n", (unsigned)value);
}

static void cmd_reset_reason(const command_t *command)
{
    uint32_t value;
    if(platform_watchdog_caused_reset(&value))
        printf("reset: watchdog, restarts %u\n", (unsigned)value);
    else
        printf("reset: other, restarts %u\n", (unsigned)value);
}

static void block_stats(const uint16_t *samples, uint32_t count, float *mean, float *rms, float *peak)
{
    uint32_t sum = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        sum += samples[i];
    }
    int32_t center = (int32_t)(sum / count);

    uint64_t squares = 0;
    uint32_t largest = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        int32_t deviation = (int32_t)samples[i] - center;
        uint32_t size = deviation < 0 ? -deviation : deviation;
        squares += (uint64_t)(deviation * deviation);
        if (size > largest)
        {
            largest = size;
        }
    }

    *mean = (float)sum / count;
    *rms = sqrtf((float)squares / count);
    *peak = (float)largest;
}

static void print_mic_bias(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    float mean;
    float rms;
    float peak;

    block_stats(samples, count, &mean, &rms, &peak);
    printf("mic: bias %.3f V\n", platform_adc_volts(mean));
}

static void print_mic_level(const uint16_t *samples, uint32_t count, uint32_t time_us)
{
    float mean;
    float rms;
    float peak;

    block_stats(samples, count, &mean, &rms, &peak);
    printf("mic: level %.1f mV rms, peak %.1f mV, %u ms\n", platform_adc_volts(rms)*1e3, platform_adc_volts(peak)*1e3, (unsigned)(count*1000/44100));
}

static void cmd_mic_bias(const command_t *command)
{
    if(platform_adc_busy() || !block_task_start(4410, &print_mic_bias))
    {
        printf("error: adc busy\n");
        return;
    }
}

static void cmd_mic_level(const command_t *command)
{
    uint32_t value;

    if(command->argc == 0)
        value = 100;
    else if ((command->argc != 1) || !parse_u32(command->argv[0], &value))
    {
        printf("error: usage mic_level [<ms>]\n");
        return;
    }

    uint64_t value_ = value*44100/1000;

    if((value_==0) || (value_>8192))
    {
        printf("error: ms must be 1..185\n");
        return;
    }

    if(platform_adc_busy() || !block_task_start(value_, &print_mic_level))
    {
        printf("error: adc busy\n");
        return;
    }
}