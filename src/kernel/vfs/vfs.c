#include <stdint.h>
#include "../utils/string.h"
#include "vfs.h"
#include "../hardware/devmgr.h"
#include "../system/memory.h"
#include "ramdisk.h"

dev_node_t *device_graph = 0;

dev_node_t *dev_graph_search(char *name, dev_node_t* basenode){
    char *nameptr_top = name;
    while(*nameptr_top != '/' || *nameptr_top == 0){
        nameptr_top++;
    }
    for(int i = 0; i < basenode->children_c; i++){
        dev_node_t *tmpnode = dev_graph_search(nameptr_top+1, basenode->children[i]);
        if(tmpnode){
            return tmpnode;
        }
    } 
    return 0;
}
dev_node_t *get_mount_from_path(char *name){
    dev_node_t *top = device_graph;
    dev_node_t *ret = 0;
    uint32_t nameptr = 1;
    while(name[nameptr] != '/'){
        if(!name[nameptr]){
            break;
        }
        nameptr++;
    }
    int child = 0;
    while(child < top->children_c){
        if(!memcpy(name, top->children[child]->mount, nameptr)){
            ret = top->children[child];
            top = top->children[child];
            name += nameptr + 1;
            nameptr = 0;
            while(name[nameptr] != '/'){
                if(!name[nameptr]){
                    break;
                }
                nameptr++;
            }
            child = 0;
        }
        child++;
    }
    return ret;
}

void mount(char *point, char *device, char *parent){
    dev_node_t *node = get_mount_from_path(parent);
    dev_node_t *new = valloc(4096, PG_ATTR_GLOBAL);
    new->mount = point;
    new->name = device;
    node->children[node->children_c++] = new;
}

void create_tmpfile(char *name, uint64_t *buffer, uint8_t type){
    if(type == USTAR_FTYPE_BLOCKDEV){
        //create a new node on the mount graph
        
    }
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

void create_device(char *mount, char *name, device_t *devinfo){
    char *buffer = valloc(512, PG_ATTR_GLOBAL);
    memcpy(buffer, devinfo, sizeof(device_t));
    int type = ((devinfo->type == DEV_TYPE_DISK) * USTAR_FTYPE_BLOCKDEV);
    write_tmpfile(name, (void *)buffer, type);
    vfree(buffer);
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

void vfs_init(){
    device_graph = valloc(sizeof(dev_node_t), PG_ATTR_GLOBAL);
    device_graph->mount = 0;
    device_graph->children_c = 0;
}