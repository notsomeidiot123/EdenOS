#pragma once
#include <stdint.h>

enum DeviceType{
    DEV_TYPE_INVALID,
    DEV_TYPE_DISK,      //usb drives fall under here
    DEV_TYPE_USB,       //this is only for misc. usb devices, like an Oculus Quest, Wacom tablet, etc
    DEV_TYPE_KEYBOARD,  //for keyboard input | any type of connector, including usb
    DEV_TYPE_MOUSE,     //for mouse input    | any type of connector, including usb
    DEV_TYPE_PCI,
    DEV_TYPE_MONITOR
};

enum DiskType{
    DEV_DISK_TYPE_INVALID,
    DEV_DISK_TYPE_RAM,
    DEV_DISK_TYPE_PATA
};

typedef struct device_s{
    enum DeviceType type;
    uint64_t devid;
    uint32_t subtype;
    uint64_t (*write)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start);
    uint64_t (*read)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start);
}device_t;


uint64_t register_device(enum DeviceType type, int sub_type, uint64_t (*read)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start), uint64_t (*write)(uint32_t *buffer, uint64_t size, uint64_t count, uint64_t start));
void deregister_device(uint64_t devid);
uint32_t get_device_by_type(enum DeviceType type, device_t **buffer);
void devmgr_init();