#include "hardware/devmgr.h"
#include "vfs/ramdisk.h"
#include "hardware/serial.h"
#include "system/idt.h"
#include "system/log.h"
#include "system/memory.h"

void kmain(kernel_info_t *kinfo) {
    serial_init();
    log_init(0);
    kprintf("Getting Memory Map!\n");
    memory_init(kinfo);
    kprintf("Initializing Required System Elements\n");
    kprintf("Paging:\n\tPAE: False\nInitializing IDT:\n");
    idt_init();
    kprintf("Starting Service EOS.devmgr:\n");
    devmgr_init();
    kprintf("Starting Service EOS.ramdisk\n");
    ramdisk_init(kinfo);
    kprintf("{DEBUG} FINISHED!\n");
}