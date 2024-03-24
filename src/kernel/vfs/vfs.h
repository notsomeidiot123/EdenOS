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
    USTAR_FTYPE_NAMED_PIPE
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

typedef struct dev_tree_node_s{
    char *mount;
    char *name;
    uint32_t children_c;
    struct dev_tree_node_s *children[];
}dev_node_t;

uint32_t write_tmpfile(char *name, uint64_t *buffer, int type);
void vfs_init();
void create_device(char *mount, char *name, device_t *devinfo);