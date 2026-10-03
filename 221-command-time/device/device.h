#pragma once

#define DEVICE_NAME "es-cmd-usb"
#define FIRMWARE_VERSION "1.1.0"

#define DEVICE_PROJECT "221-command-time"
#define DEVICE_REPO "https://github.com/dordienkodv/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

#include <stdint.h>

typedef void (*info_t)(void);

struct info_t
{
    uint32_t version;
    char name[13];
    uint8_t revision;
};

extern struct info_t device_card;
void device_card_init(void);

void device_info(void);
void dev_info(void);