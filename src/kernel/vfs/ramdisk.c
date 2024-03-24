#include "ramdisk.h"
#include "../system/log.h"
#include "../hardware/devmgr.h"
#include "../system/memory.h"
#include "../utils/string.h"


uint32_t *ramdisk = 0;
uint64_t ramdisk_size = 0;
uint64_t ramdisk_write(uint64_t *buffer, uint64_t count, uint64_t cmd, uint64_t start){
    if(start * 512 + count * 512 > ramdisk_size){
        count -= (start * 512 + count * 512 ) - ramdisk_size;
        count /= 512;
    }
    memcpy(ramdisk + (start * 512), buffer, count * 512);
    return count;
}
uint64_t ramdisk_read(uint64_t *buffer, uint64_t count, uint64_t cmd, uint64_t start){
    if(start * 512 + count * 512 > ramdisk_size){
        count -= (start * 512 + count * 512 ) - ramdisk_size;
        count /= 512;
    }
    memcpy(buffer, ramdisk + (start * 512),count * 512);
    return count;
}
void ramdisk_init(kernel_info_t *kinfo){
    if(kinfo->ramdisk_ptr){
        ramdisk = (uint32_t *)kinfo->ramdisk_ptr;
        ramdisk_size = kinfo->ramdisk_sz;
    }
    kprintf("\tAllocating Ramdisk\n");
    ramdisk = valloc_pages(1024, PG_ATTR_GLOBAL | PG_ATTR_WRITE);
    kprintf("\tRamdisk Address: %0x\n", ramdisk);
    ramdisk_size = 4096 * 1024;
    ramdisk[257/4] = 0;
    register_device(DEV_TYPE_DISK, DEV_DISK_TYPE_RAM, ramdisk_read, ramdisk_write);
}