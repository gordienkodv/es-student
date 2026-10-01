#include "memory.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "device.h"
#include "command.h"

#define RP2040_SRAM_SIZE_BYTES  (264 * 1024U)  // 270336
#define RP2040_ROM_SIZE_BYTES   (16  * 1024U)  // 16384

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;

uint32_t data_variable = 100;
uint32_t bss_variable;

int main(void);

static void row(const char* name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n",
        name, (unsigned)start, (unsigned)end, (unsigned)(end - start));
}

void mem_info(void)
{
    printf("%-10s %-10s %-10s %8s\n", "area", "start", "end", "size");

    row("flash", (uintptr_t)XIP_BASE, (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("sram", (uintptr_t)SRAM_BASE, (uintptr_t)SRAM_BASE + RP2040_SRAM_SIZE_BYTES);
    row("rom", (uintptr_t)ROM_BASE, (uintptr_t)ROM_BASE + RP2040_ROM_SIZE_BYTES);

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, (uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES);
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    row("data flash", (uintptr_t)&__etext, (uintptr_t)&__flash_binary_end);
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    printf("\ntotal\n");

    unsigned image_size = (unsigned)((uintptr_t)&__flash_binary_end - (uintptr_t)&__flash_binary_start);
    unsigned boot2_size = (unsigned)((uintptr_t)&__boot2_end__ - (uintptr_t)&__boot2_start__);
    unsigned text_size = (unsigned)((uintptr_t)&__etext - (uintptr_t)&__boot2_end__);
    unsigned data_size = (unsigned)((uintptr_t)&__flash_binary_end - (uintptr_t)&__etext);
    printf("  flash image    %8u = boot2 %u + text %u + data %u\n", image_size, boot2_size, text_size, data_size);

    unsigned free_size = (unsigned)((uintptr_t)XIP_BASE + PICO_FLASH_SIZE_BYTES - (uintptr_t)&__flash_binary_end);
    printf("  flash free   %8u of %u\n", free_size, PICO_FLASH_SIZE_BYTES);

    unsigned data_ram_size = (unsigned)((uintptr_t)&__data_end__ - (uintptr_t)&__data_start__);
    unsigned bss_size = (unsigned)((uintptr_t)&__bss_end__ - (uintptr_t)&__bss_start__);
    unsigned ram_used = data_ram_size + bss_size;
    printf("  ram used       %8u = data %u + bss %u\n", ram_used, data_ram_size, bss_size);

    unsigned heap_size = (unsigned)((uintptr_t)&__HeapLimit - (uintptr_t)&__bss_end__);
    unsigned stack_size = (unsigned)((uintptr_t)&__StackTop - (uintptr_t)&__StackBottom);

    printf("  ram free       %8u for heap and %u for stack\n", heap_size, stack_size);
}


void fw_info(void)
{
    data_variable++;
    bss_variable++;

    uint16_t* main_code = (uint16_t*)((uintptr_t)main & ~1u);
    uint16_t* fw_info_code = (uint16_t*)((uintptr_t)fw_info & ~1u);

    uint32_t stack_variable = 1946;
    uint32_t* heap_variable = malloc(sizeof(uint32_t));

    if (heap_variable != NULL)
    {
        *heap_variable = 1951;
    }

    printf("%-16s %-10s %s\n", "object", "address", "value");
    printf("%-16s 0x%08x 0x%04x\n", "main", (unsigned)&main, (unsigned)*main_code);
    printf("%-16s 0x%08x 0x%04x\n", "fw_info", (unsigned)&fw_info, (unsigned)*fw_info_code);
    printf("%-16s 0x%08x\n", "commands", (unsigned)&commands);
    
    for (uint i = 0; i < command_count; i++)
    {
        printf("- %-14s 0x%08x\n", commands[i].name, (unsigned)&commands[i].handler);
    }

    printf("%-16s 0x%08x %s\n", "DEVICE_PROJECT", (unsigned)&DEVICE_PROJECT, DEVICE_PROJECT);
    printf("%-16s 0x%08x %s\n", "DEVICE_BOARD", (unsigned)&DEVICE_BOARD, DEVICE_BOARD);
    printf("%-16s 0x%08x %d\n", "data_variable", (unsigned)&data_variable, data_variable);
    printf("%-16s 0x%08x %d\n", "bss_variable", (unsigned)&bss_variable, bss_variable);
    printf("%-16s 0x%08x %d\n", "stack_variable", (unsigned)&stack_variable, stack_variable);
    printf("%-16s 0x%08x %d\n", "heap_variable", (unsigned)&heap_variable, *heap_variable);

    free(heap_variable);
}