#pragma once
#include <stdint.h>

typedef struct cpu_registers_s{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, edx, ecx, ebx, eax;
    uint32_t int_no, ecode;
    uint32_t eip, cs, eflags, esp, ss;
}regs_t;

#define IDT_TRAP_USER   0xBF
#define IDT_INT_USER    0xBE
#define IDT_TRAP_KERNEL 0x8F
#define IDT_INT_KERNEL  0x8E

enum IDTFlags{
    IDT_PRESENT = 0x80,
    IDT_TASK        = 0x5,
    IDT_INT16       = 0x6,
    IDT_TRAP16      = 0x7,
    IDT_INT32       = 0x0e,
    IDT_TRAP32      = 0x0f,
    IDT_DPL_USER    = 0x30,
    IDT_DPL_KERNEL  = 0x00,
    IDT_DPL_PRIVL   = 0x10,
};

typedef struct idt_entry_s{
    uint16_t offset_low;
    uint16_t code_seg;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
}idt_entry_t;
typedef struct idt_descriptor_s{
    uint16_t size;
    idt_entry_t **offset;
}__attribute__((packed))idt_descriptor_t;

#define MASTER_PIC 0x20
#define SLAVE_PIC 0xa0
#define PIC_MASTER_COMMAND MASTER_PIC
#define PIC_SLAVE_COMMAND SLAVE_PIC
#define PIC_MASTER_DATA MASTER_PIC + 1
#define PIC_SLAVE_DATA SLAVE_PIC + 1
#define PIC_EOI 0x20

void kernel_panic(char *msg, regs_t *regs);

#define PANIC(msg) kernel_panic(msg, 0);

void idt_init();