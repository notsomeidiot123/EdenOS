#include <stdint.h>
#include "../utils/string.h"
#include "vfs.h"
#include "../hardware/devmgr.h"
#include "../system/memory.h"
#include "ramdisk.h"

filesystem_t **mount_list = 0;

void create_tmpfile(char *name, uint64_t *buffer, uint8_t type){
    uint32_t file = 0;
    tmpfile_t tmpfile = {};
    ramdisk_read((void *)&tmpfile, 2, 0, file * 2);
    while(!strcmp(tmpfile.ustar_sig, "USTAR")){
        ramdisk_read((uint64_t *)&tmpfile, 2, 0, file * 2);
        file++;
    }
    memcpy(&(tmpfile.ustar_sig), "USTAR", 6);
    memcpy(&(tmpfile.ustar_ver), "00", 2);
    tmpfile.type = type;
    memcpy(&tmpfile, name, strlen(name));
    memcpy(tmpfile.data, buffer, 512);
    ramdisk_write((void *)&tmpfile, 2, 0, file * 2);
}
//just to unify all calls to the driver
//type can be any value, it's garbage
uint32_t read_tmpfile(char *name, uint64_t *buffer, int type){
    uint32_t file = 0;
    tmpfile_t tmpfile = {};
    ramdisk_read((void *)&tmpfile, 2, 0, file * 2);
    while(!strcmp(tmpfile.ustar_sig, "USTAR")){
        if(!strcmp(tmpfile.name, name)){
            memcpy(buffer, tmpfile.data, 512);
            return 512;
        }
        file++;
        ramdisk_read((void *)&tmpfile, 2, 0, file * 2);
    }
    return 0;
}

uint32_t write_tmpfile(char *name, uint64_t *buffer, int type){
    uint32_t file = 0;
    tmpfile_t tmpfile = {};
    ramdisk_read((void *)&tmpfile, 2, 0, file * 2);
    while(!strcmp(tmpfile.ustar_sig, "USTAR")){
        if(!strcmp(tmpfile.name, name)){
            memcpy(tmpfile.data, buffer, 512);
            ramdisk_write((void *)&tmpfile, 2, 0, file);
            return 512;
        }
        file++;
        ramdisk_read((void *)&tmpfile, 2, 0, file * 2);
    }
    create_tmpfile(name, buffer, type);
    return 512;
}

void mount(void *read, void *write, device_t *device, char position){
    if(!mount_list[position]){
        filesystem_t *fs = valloc(512, PG_ATTR_GLOBAL);
        fs->device = device;
        fs->read = read;
        fs->write = write;
        mount_list[position] = fs;
    }
}
void unmount(char pos){
    if(mount_list[pos]){
        vfree(mount_list[pos]);
        mount_list[pos] = 0;
    }
}
//returns characters read from file filename
uint64_t read(char *filename, char *buffer, uint64_t size){
    if(filename[1] != ':'){
        return 0;
    }
    char mount = filename[0];
    return mount_list[mount]->read(filename + 2, buffer, size, mount_list[mount]->device);
}
uint64_t write(char *filename, char *buffer, uint64_t size){
    if(filename[1] != ':'){
        return 0;
    }
    char mount = filename[0];
    return mount_list[mount]->write(filename + 2, buffer, size, mount_list[mount]->device);
}

void vfs_init(){
    mount_list = valloc(26 * sizeof(device_t *), PG_ATTR_GLOBAL);
}