#include "idt.h"
#include "log.h"
#include "../hardware/io.h"
#include "memory.h"
#include <stdint.h>
#include <stdnoreturn.h>
extern void _isr_common_stub();

extern void _isr0();
extern void _isr1();
extern void _isr2();
extern void _isr3();
extern void _isr4();
extern void _isr5();
extern void _isr6();
extern void _isr7();
extern void _isr8();
extern void _isr9();
extern void _isr10();
extern void _isr11();
extern void _isr12();
extern void _isr13();
extern void _isr14();
extern void _isr15();
extern void _isr16();
extern void _isr17();
extern void _isr18();
extern void _isr19();
extern void _isr20();
extern void _isr21();
extern void _isr22();
extern void _isr23();
extern void _isr24();
extern void _isr25();
extern void _isr26();
extern void _isr27();
extern void _isr28();
extern void _isr29();
extern void _isr30();
extern void _isr31();

extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();
extern void load_idt(idt_descriptor_t *idt);

idt_entry_t idt[256] = {0};
idt_descriptor_t idt_desc = {0};
void enable_irqs(){
    asm volatile("sti");
}
void disable_irqs(){
    asm volatile("cli");
}

void mask_pic(uint8_t pic, uint8_t mask){
    outb(pic, mask);
}

regs_t *(*irq_handlers[16])(regs_t *regs) = {0};

void irq_register_callback(int32_t irq, regs_t *(*callback)(regs_t *regs)){
    irq_handlers[irq] = callback;
}

void kernel_panic(char *msg, regs_t *regs){
    kprintf("Kernel Panic! %s", msg);
    if(!regs){
        for(;;);
    }
    kprintf("[[DEBUG]]\n");
    kprintf("EAX: %h EBX: %h ECX: %h EDX: %h\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
    kprintf("ESP: %h EBP: %h EDI: %h ESI: %h\n", regs->esp, regs->ebp, regs->edi, regs->esi);
    kprintf("CS: %h DS: %h EFLAGS: %h\n", regs->cs, regs->ds, regs->eflags);
    kprintf("EIP: %h CR3: %h CR2: %h", regs->eip, get_cr3(), get_cr2());
    kprintf("Error Code: %h\nException %h", regs->ecode, regs->int_no);
    for(;;);
}

void _irq_handler(regs_t *regs){
    int int_no = regs->int_no - 32;
    if(irq_handlers[int_no]){
        regs = irq_handlers[int_no](regs);
    }
    if(int_no >= 8){
        outb(SLAVE_PIC, PIC_EOI);
    }
    outb(MASTER_PIC, PIC_EOI);
    return;
}

void _isr_handler(regs_t *regs){
    if(regs->int_no == 8 || regs->int_no == 18){
        kprintf("Fatal Error: Aborting\n");
        for(;;);
    }
    //TODO: REPLACE MAGIC VALUE WITH MACRO FROM GDT
    if(regs->cs == 0x8){
        kernel_panic("A Fatal Error has occured in Kernel Space", regs);
    }
    return;
}

void idt_set_gate(int index, void (*offset), enum IDTFlags flags){
    idt[index].offset_low = (uint64_t)offset & 0xffff;
    idt[index].offset_high = ((uint64_t)offset >> 16) & 0xffff;
    idt[index].flags = flags | 0x80;
    idt[index].code_seg = 0x8;
}

void idt_init(){
    kprintf("\tException Handlers: ");
    
    // idt_set_gate(i, _isr0 + ((_isr1 - _isr0) * i), IDT_INT_KERNEL);
    idt_set_gate(0, _isr0, IDT_INT_KERNEL);
    idt_set_gate(1, _isr1, IDT_INT_KERNEL);
    idt_set_gate(2, _isr2, IDT_INT_KERNEL);
    idt_set_gate(3, _isr3, IDT_INT_USER);
    idt_set_gate(4, _isr4, IDT_INT_KERNEL);
    idt_set_gate(5, _isr5, IDT_INT_KERNEL);
    idt_set_gate(6, _isr6, IDT_INT_KERNEL);
    idt_set_gate(7, _isr7, IDT_INT_KERNEL);
    idt_set_gate(8, _isr8, IDT_INT_KERNEL);
    idt_set_gate(9, _isr9, IDT_INT_KERNEL);
    idt_set_gate(10, _isr10, IDT_INT_KERNEL);
    idt_set_gate(11, _isr11, IDT_INT_KERNEL);
    idt_set_gate(12, _isr12, IDT_INT_KERNEL);
    idt_set_gate(13, _isr13, IDT_INT_KERNEL);
    idt_set_gate(14, _isr14, IDT_INT_KERNEL);
    idt_set_gate(15, _isr15, IDT_INT_KERNEL);
    idt_set_gate(16, _isr16, IDT_INT_KERNEL);
    idt_set_gate(17, _isr17, IDT_INT_KERNEL);
    idt_set_gate(18, _isr18, IDT_INT_KERNEL);
    idt_set_gate(19, _isr19, IDT_INT_KERNEL);
    idt_set_gate(20, _isr20, IDT_INT_KERNEL);
    idt_set_gate(21, _isr21, IDT_INT_KERNEL);
    idt_set_gate(22, _isr22, IDT_INT_KERNEL);
    idt_set_gate(23, _isr23, IDT_INT_KERNEL);
    idt_set_gate(24, _isr24, IDT_INT_KERNEL);
    idt_set_gate(25, _isr25, IDT_INT_KERNEL);
    idt_set_gate(26, _isr26, IDT_INT_KERNEL);
    idt_set_gate(27, _isr27, IDT_INT_KERNEL);
    idt_set_gate(28, _isr28, IDT_INT_KERNEL);
    idt_set_gate(29, _isr29, IDT_INT_KERNEL);
    idt_set_gate(30, _isr30, IDT_INT_KERNEL);
    idt_set_gate(31, _isr31, IDT_INT_KERNEL);
    
    kprintf("SET\n\tIDT Address: %0x\n\tIDT Descriptor: %0x\n\tPeripheral Interrupts: ", &idt, &idt_desc);
    for(int i = 0; i < 16; i++){
        idt_set_gate(i + 32, (void *)(_irq0 + (_irq1 - _irq0) * i), IDT_INT_KERNEL);
    }
    kprintf("SET\nInitializing PIC:\n");
    
    idt_desc.size = sizeof(idt_entry_t) * 256-1;
    idt_desc.offset = (idt_entry_t **)&idt;
    
    outb(MASTER_PIC, 0x11 );
    outb(SLAVE_PIC, 0x11);
    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_SLAVE_DATA, 0x28);
    outb(PIC_MASTER_DATA, 4);
    outb(PIC_SLAVE_DATA, 2);
    outb(PIC_MASTER_DATA, 1);
    outb(PIC_SLAVE_DATA, 1);

    kprintf("\tRemapped PICs\nLoading IDT\n");
    load_idt(&idt_desc);
    kprintf("Enabling Interrupts\n");
    kprintf("Remasking PICs\n");
    outb(PIC_MASTER_DATA, 0);
    outb(PIC_SLAVE_DATA, 0);
    outb(PIC_MASTER_COMMAND, PIC_EOI);
    outb(PIC_SLAVE_COMMAND, PIC_EOI);
    enable_irqs();
}