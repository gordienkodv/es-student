#pragma once

#define DEVICE_NAME "firmware"
#define FIRMWARE_VERSION "1.1.0"

#define DEVICE_PROJECT "231-firmware-template"
#define DEVICE_REPO "https://github.com/dordienkodv/es-student"

#ifndef DEVICE_BOARD
#define DEVICE_BOARD "unknown"
#endif

#define DEVICE_SERIAL_SIZE 17

#include "pico/types.h"

typedef struct {
    char* board;
    char serial[DEVICE_SERIAL_SIZE];
    uint16_t chip_manufacturer;
    uint16_t chip_part;
    uint8_t chip_revision;
    char* sdk_version;
} device_info_t;

extern device_info_t info;

void device_get_info(device_info_t *info);