#pragma once

#include <stdint.h>

#define DEVICE_SERIAL_SIZE 17

typedef struct {
    uint16_t chip_manufacturer;
    uint16_t chip_part;
    char* board;
    char* sdk_version;
    char serial[DEVICE_SERIAL_SIZE];
    uint8_t chip_revision;
} device_info_t;

void device_get_info(device_info_t *info);
