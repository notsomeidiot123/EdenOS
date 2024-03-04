#include "devmgr.h"
#include "../system/memory.h"
#include "../system/idt.h"
#include "../system/log.h"

#define MAX_DEVICES 512

device_t **devices = 0;
uint64_t next_devid = 1;

uint64_t devmgr_gendevid(){
    next_devid ^= next_devid << 13;
    next_devid ^= next_devid >> 17;
    next_devid ^= next_devid << 5;
    return next_devid;
}

uint64_t register_device(enum DeviceType type, int sub_type, uint64_t (*read)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start), uint64_t (*write)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start)){
    device_t **devlist = devices;
    int i = 0;
    for(; i < MAX_DEVICES; i++){
        if(!devlist[i]){
            break;
        }
    }
    *devlist = valloc(sizeof(device_t), PG_ATTR_GLOBAL);
    device_t *dev = devlist[i];
    dev->type = type;
    dev->subtype = sub_type;
    dev->write = write;
    dev->read = read;
    dev->devid = devmgr_gendevid();
    return dev->devid;
}
uint32_t get_device_by_type(enum DeviceType type, device_t **buffer){
    uint32_t bp = 0;
    for(int i = 0; i < MAX_DEVICES; i++){
        if(devices[i] && devices[i]->type == type){
            buffer[bp++] = devices[i];
        }
    }
    return bp;
}
void deregister_device(uint64_t devid){
    device_t **devlist = devices;
    for(int i = 0; i < MAX_DEVICES; i++){
        if(devlist[i] && devlist[i]->devid == devid){
            vfree(devlist[i]);
            devlist[i] = 0;
        }
    }
}

void devmgr_init(){
    printf("\tAllocating space for device_list\n");
    devices = valloc(sizeof(device_t *) * MAX_DEVICES, PG_ATTR_GLOBAL);
    if(devices == 0){
        PANIC("Could not initialize device manager!\n");
    }
}