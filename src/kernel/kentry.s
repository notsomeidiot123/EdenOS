bits 32
extern kmain
global _start
global test_eax
_start:
    call kmain
jmp $

test_eax:
    mov eax, [esp + 4]
    jmp $