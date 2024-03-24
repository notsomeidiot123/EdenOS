#pragma once
#include <stdint.h>
#include "../system/memory.h"

uint64_t ramdisk_read(uint64_t *buffer, uint64_t count, uint64_t cmd, uint64_t start);;
uint64_t ramdisk_write(uint64_t *buffer, uint64_t count, uint64_t cmd, uint64_t start);;
void ramdisk_init(kernel_info_t *kinfo);