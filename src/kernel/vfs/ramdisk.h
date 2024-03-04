#pragma once
#include <stdint.h>

uint64_t ramdisk_read(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start);;
uint64_t ramdisk_write(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start);;
void ramdisk_init();