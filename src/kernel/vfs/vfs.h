#pragma once
#include <stdint.h>
#include "../hardware/devmgr.h"
enum USTARFileTypes{
    USTAR_FTYPE_NORMAL,
    USTAR_FTYPE_HARD,
    USTAR_FTYPE_SYM,
    USTAR_FTYPE_CHARDEV,
    USTAR_FTYPE_BLOCKDEV,
    USTAR_FTYPE_DIRECTORY,
    USTAR_FTYPE_NAMED_PIPE,
    EOS_FTYPE_MOUNT,
};

typedef struct USTAR_file_s{
    char name[100];
    uint64_t file_mode;
    uint64_t onuid;//owner numeric user id
    uint64_t gnuid;//group numeric user id
    char file_size[12];//octal string
    char last_mod_time[12];//octal string
    uint64_t header_checksum;
    uint8_t type;
    char link_name[100];
    char ustar_sig[6];//'U', 'S', 'T', 'A', 'R', NUL
    char ustar_ver[2];//"00"
    char ouname[32];//owner username
    char ogname[32];//owner group name
    uint64_t devnum_major;
    uint64_t devnum_minor;
    char fname_prefix[155];
    char data[512];
}tmpfile_t;

typedef struct file_descriptor_s{
    char *filename;
    uint64_t position;
    uint64_t size;
    uint8_t lock;
    uint8_t permissions;
    void *_fs_file_descriptor;/*for use by individual filesystems to store
    information about the file itself*/
}file_t;

typedef struct filesystem_s{
    uint64_t (*read)(char *fname, char *buffer, uint64_t size, device_t *device);
    uint64_t (*write)(char *fname, char *buffer, uint64_t size, device_t *device);
    device_t *device;
}filesystem_t;

uint32_t write_tmpfile(char *name, uint64_t *buffer, int type);
void vfs_init();
void create_device(char *mount, char *name, device_t *devinfo);