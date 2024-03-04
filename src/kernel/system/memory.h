#include <stdint.h>

enum MMAP_TYPES{
    MMAP_TYPE_USABLE = 1,
    MMAP_TYPE_RESERVED,
    MMAP_TYPE_ACPI_RECLAIM,
    MMAP_TYPE_ACPI_NVS,
    MMAP_TYPE_BAD
};

typedef struct mmap_s{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t extended;
}mmap_t;

typedef struct kernel_info{
    void *mmap;//update this
    uint32_t mmap_count;
    void *vbe_info;
    void *edid_info;
    void *vbe_mode_info;
    uint32_t vbe_mode_count;
    uint16_t vbe_x_res;
    uint16_t vbe_y_res;
    uint16_t mon_x_res;
    uint16_t mon_y_res;
    uint16_t vbe_mode_id;
    uint16_t color_mode;//deprecated
    uint8_t vbe_bpp;
    void *vbe_framebuffer;
    uint8_t cpuid_not_supported;
    uint8_t *ramdisk_ptr;
    uint64_t ramdisk_sz;
    uint8_t *pmbm; //physical memory bitmap, id mapped
    uint32_t bitmap_size;
}__attribute__((packed))kernel_info_t;
enum PAGE_ATTRIBUTES{
    PG_ATTR_PRESENT =   0x001,
    PG_ATTR_WRITE =     0x002,
    PG_ATTR_USER =      0x004,
    PG_ATTR_WT =        0x008,
    PG_ATTR_CD =        0x010,
    PG_ATTR_ACCESSED =  0x020,
    PG_ATTR_DIRTY =     0x040,
    PG_ATTR_PAT =       0x080,
    PG_ATTR_GLOBAL =    0x100,
    PG_ATTR_LINK_NEXT = 0x200,
    PG_ATTR_LINK_LAST = 0x400,
    PG_ATTR_USED      = 0x800
};

typedef struct page_dir_entry_s{
    uint32_t present:1;
    uint32_t read_write:1;
    uint32_t user:1;
    uint32_t write_thru:1;
    uint32_t cache_disable:1;
    uint32_t accessed:1;
    uint32_t unused:1;
    uint32_t page_size:1;
    uint32_t unused_0:4;
    uint32_t addr:20;
}__attribute__((packed))pde_t;
typedef struct page_table_entry_s{
    uint32_t present:1;
    uint32_t read_write:1;
    uint32_t user:1;
    uint32_t write_thru:1;
    uint32_t cache_disable:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t attr_table:1;
    uint32_t global:1;
    uint32_t link_next:1;
    uint32_t link_last:1;
    uint32_t used:1;
    uint32_t addr:20;
}pte_t;

enum MEMORY_LIST_VALS{
    ML_FREE,
    ML_USED,
    ML_LINK_NEXT=4,
    ML_LINK_LAST=8
};

void memory_init(kernel_info_t *kinfo);
void unmap(void *vaddr);
int32_t map(uint32_t paddr, void *vaddr, uint16_t attributes);

uint64_t get_cr3();
void set_cr3(uint32_t pd_addr);
uint64_t get_cr2();

void *valloc(uint32_t size, uint32_t attributes);
void vfree(void *vaddr);