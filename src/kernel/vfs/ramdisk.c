#include "ramdisk.h"
#include "../system/log.h"
#include "../hardware/devmgr.h"
#include "../system/memory.h"
#include <sys/types.h>
uint32_t *ramdisk = 0;
uint64_t ramdisk_write(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start){
    if((size/512) + 1 + (start/512) > 4096){
        return 0;
    }
    for(uint64_t i = start/512; i < size; i++){
        ramdisk[i] = buffer[i - (start/512)];
        if((start/512) + i > 4096 * 512){
            return i - (start/512);
        }
    }
    return size;
}
uint64_t ramdisk_read(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start){
    if((size/512) + 1 + (start/512) > 4096){
        return 0;
    }
    for(uint64_t i = start/512; i < size; i++){
        buffer[i - (start/512)] = ramdisk[i];
        if((start/512) + i > 4096 * 512){
            return i - (start/512);
        }
    }
    return size;
}
void ramdisk_init(){
    printf("\tAllocating Ramdisk\n");
    ramdisk = valloc(2 * 1024 * 1024, PG_ATTR_GLOBAL);
    register_device(DEV_TYPE_DISK, DEV_DISK_TYPE_RAM, ramdisk_read, ramdisk_write);
}