#include "hardware/devmgr.h"
#include "vfs/ramdisk.h"
#include "hardware/serial.h"
#include "system/idt.h"
#include "system/log.h"
#include "system/memory.h"

void kmain(kernel_info_t *kinfo) {
    serial_init();
    log_init(0);
    printf("Getting Memory Map!\n");
    memory_init(kinfo);
    printf("Initializing Required System Elements\n");
    printf("Paging:\n\tPAE: False\nInitializing IDT:\n");
    idt_init();
    printf("Starting Service EOS.devmgr:\n");
    devmgr_init();
    printf("Starting Service EOS.ramdisk\n");
    ramdisk_init();
    printf("{DEBUG} FINISHED!\n");
}