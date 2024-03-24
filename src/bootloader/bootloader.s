org 0x7c00
bits 16

start:
    mov bx, ds
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    
    mov [part_off], si
    mov [part_seg], bx
    mov [boot_disk], dl
    mov esp, 0x7a00
    mov ebp, 0x7a00
    
    xor edx, edx
    jmp 0:check_a20
check_a20:
    ;edx MUST BE PRESERVED
    cmp edx, 3
    je no_a20
    mov ebx, 0x112345
    mov eax, 0x012345
    mov word [esi], 0
    mov word [edi], 0
    mov word [edi], 0xaa55
    cmpsb
    jne en_unreal
    call enable_a20
    jmp check_a20
    jmp $
no_a20:
    ; mov bx, strtab.no_a20
    ; call Strs.printr
    mov ax, 0x410e
    int 0x10
    jmp $
en_unreal:
    mov eax, 0x5a5aa5a5
    cli
    push ds
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    ; jmp $
    jmp 0x8:.tpmode
bits 32
    .tpmode:
        mov bx, 0x10
        mov ds, bx
        and al, 0xfe
        mov cr0, eax
        jmp 0:load_boot
bits 16
load_boot:
    pop ds
    sti
    
    mov es, [part_seg]
    mov di, [part_off]
    mov eax, [es:di + 0x8]
    inc eax
    mov [DAP.lba_low], eax
    lea si, DAP
    mov dl, [boot_disk]
    mov ah, 0x42
    xor al, al
    int 0x13
    jc .err
;DEBUG
    mov bx, 0xf42
    mov eax, 0xb8000
    mov [ds:eax], bx
;end_debug
    jmp part_2

    .err:
        mov si, strings.read_err
        call print.string
    .hltlp:
        hlt
        jmp .hltlp
strings:
    .read_err: db "[Error 0x13] Could not read disk! (Fatal)", 0xa, 0xd, 0
    .memerr: db "[Error 0x15] Could not fetch memory map! (Fatal)", 0xa, 0xd, 0
DAP:
    .size:      db 0x10
    .res:       db 0
    .sectors:   dw 7
    .offset:    dw 0x7e00
    .segment:   dw 0
    .lba_low:   dd 0
    .lba_high:  dd 0

gdt_desc:
    .size: dw unreal_gdt.end - unreal_gdt - 1
    .offset: dd unreal_gdt
unreal_gdt:
    .NULL:
        dd 0
        dd 0
    .CODE:
        dw 0xffff
        dw 0
        db 0
        db 0b10011010
        db 0
        db 0
    .DATA:
        dw 0xffff
        dw 0
        db 0
        db 0b10010010
        db 0b11001111
        db 0
    .end:
print:
    .string:
        ;call with string in si
        push eax
        lodsb
        cmp al, 0
        je .ret
        mov ah, 0x0e
        int 0x10
        jmp .string
    .char:
        push dword 0
        mov ah, 0xe
        int 0x10
    .ret:
        pop eax
        ret

enable_a20: ;ADAPTED FROM PREVIOUS ATTEMPT
    cmp edx, 0
    jmp .bios
    cmp edx, 1
    jmp .kbdc
    cmp edx, 2
    jmp .fa20
    .bios:
        mov ax, 0x2401
        int 0x15
        jb .ret
        cmp ah, 0
        jb .ret
    .kbdc:
        cli
        call .wait
        mov al, 0xAD
        out 0x64, al

        call .wait
        mov al, 0xd0
        out 0x64, al

        call .wait2
        in al, 0x60
        push eax
        
        call .wait
        pop eax
        or al, 2
        out 0x60, al
        call .wait
        mov al, 0xAE
        out 0x64, al
        sti
        ret
    .wait:
        in al, 0x64
        test al, 2
        jne .wait
        ret
    .wait2:
        in al,0x64
        test al,1
        jz .wait2
        ret
    .wait3:
        mov eax, 0x100000
        .w3l0:
            cmp eax, 0
            je .ret
            dec eax
            jmp .w3l0
    .fa20:
        in al, 0x92
        or al, 2
        out 0x92, al
        jmp .wait3
    .ret:
        inc edx
        ret
    jmp $

part_seg:   dw 0
part_off:   dw 0
boot_disk:  db 0
times 510 - ($-$$) db 0
db 0x55, 0xaa

;pre-paging memory map:
;   0x0000_0000 - 0x0000_0500: Reserved for BIOS
;   0x0000_0500 - 0x0000_0800: Reserved for MBR
;   0x0000_0800 - 0x0000_7a00: Stack
;   0x0000_7a00 - 0x0000_7c00: Free (256 bytes)
;   0x0000_7c00 - 0x0000_8c00: Bootloader
;   0x0000_8c00 - 0x0000_9000: Free (1024 bytes)
;   0x0000_9000 - 0x0001_0000: VESA info & MMAP
;   0x0001_0000 - 0x0001_8000: Kernel Loading Area & Free memory
;   0x0001_8000 - 0x0008_0000: Free memory (>200kb)
;   0x0008_0000 - 0x000a_0000: EBDA
;   0x000a_0000 - 0x000c_0000: VGA video memory
;   0x000c_0000 - 0x000c_8000: Video BIOS
;   0x000c_8000 - 0x000f_0000: BIOS Expansions
;   0x000f_0000 - 0x0010_0000: BIOS
;   0x0010_0000 - 0x0020_0000: Physical Memory Map
;   0x0020_0000 - 0x00f0_0000: Kernel's physical location

KERNEL_LOAD     equ 0x00010000
KERNEL_MOVE     equ 0x00200000
KERNEL_MAPPED   equ 0xc0000000

kernel_dap:
    .size:      db 0x10
    .res:       db 0
    .sectors:   dw 0x80 ;SIZE OF KERNEL IN SECTORS
    .offset:    dw 0x0000
    .segment:   dw 0x1000
    .lba_low:   dd 0
    .lba_high:  dd 0
part_2:
;DEBUG
    mov bx, 0x0f32
    mov eax, 0xb8002
    mov [ds:eax], bx
;END_DEBUG
load_kernel:
    mov di, [part_off]
    mov es, [part_seg]
    
    mov eax, [es:di + 8]
    add eax, 0x8;bootloader is 8 sectors, kernel is right after bootloader
    mov [kernel_dap.lba_low], eax
    mov ax, 0
    mov ds, ax
    mov si, kernel_dap
    mov ah, 0x42
    
    mov dl, [boot_disk]
    clc
    int 0x13
    jc load_boot.err
    add dword [kernel_dap.lba_low], 0x80
    add word [kernel_dap.segment], 0x1000
    mov ax, 0
    mov ds, ax
    mov si, kernel_dap
    mov ah, 0x42
    
    mov dl, [boot_disk]
    clc
    int 0x13
    jc load_boot.err
    add dword [kernel_dap.lba_low], 0x80
    add word [kernel_dap.segment], 0x1000
    mov ax, 0
    mov ds, ax
    mov si, kernel_dap
    mov ah, 0x42
    
    mov dl, [boot_disk]
    clc
    int 0x13
    jc load_boot.err
reloc_kernel:
    xor ebx, ebx
    mov es, bx
    xor edx, edx
    mov eax, KERNEL_LOAD
    mov ebx, KERNEL_MOVE
    xor ecx, ecx
    
    .ml0:
        cmp ecx, 0x80*0x80
        je .ml0e
        mov edx, [eax + ecx*4]
        mov [ebx+ecx*4], edx
        inc ecx
        jmp .ml0
    .ml0e:;LESS THAN A SECOND???
;DEBUG
    mov bx, 0x0f4b
    mov eax, 0xb8004
    mov [ds:eax], bx
;END_DEBUG
get_mmap:
    xor bx, bx
    mov es, bx
    mov di, MMAP_PTR
    call gen_mmap

;DEBUG
    mov bx, 0x0f4d
    mov eax, 0xb8006
    mov [ds:eax], bx
;END_DEBUG
get_vbe:
    .check_cpuid:
        pushfd
        pushfd
        xor dword [esp], 1 << 21
        popfd
        pushfd
        pop eax
        xor eax, esp
        popfd
        and eax, 1<<21
        jz .cpuid_ns
        jmp call_vbe
    .cpuid_ns:
        mov byte [kernel_info_struct.cpuid_ns], 1
        ;actually has a lot of information... idk what i'd do without it
    ; mov ax, 0x4f02
    ; mov bx, 0x4118 ;1024*768*24 VESA
    ; int 0x10
call_vbe:
    mov di, VBE_PTR
    call gen_vbe_info
    ; jmp $
init_paging:;from this point on, no way to know what's happening... hope it goes well
    mov eax, DEFAULT_PG_DIR; store ptr to page directory in eax
    mov ebx, PAGE_TABLES; first page table is 64kb after page dir
;     ;present, write enabled
    mov ecx, 0
.l0:
    mov dword [eax + ecx * 4], 0
    cmp ecx, 0x400
    je id_map_mb1
    inc ecx
    jmp .l0
    ;clear page_dir
id_map_mb1:
    xor ecx, ecx;store page table index in ecx
    mov [ds:eax], ebx
    or dword [ds:eax], 3
    mov edx, 0x803
    .l0:
        mov [ds:ebx], edx;i hope this works. if i'm right, this will loop over the first 256 table entries (out of 1024) and map them to themselves
        
        add ebx, 4
        add edx, 4096
        inc ecx
        cmp ecx, 512
        je id_map_framebuffer
        jmp .l0
id_map_framebuffer: ;map until edge of table
    push eax
    
    mov edx, [kernel_info_struct.framebuffer]
    mov ebx, edx
    shr ebx, 22
    mov ecx, edx
    shr ecx, 12
    and ecx, 0x3ff
    ; jmp $
    mov dword [eax + ebx * 4], PAGE_TABLES + 4096 | 3
    mov esi, PAGE_TABLES + 4096
    or edx, 0x803
    .l0:
        mov [esi + ecx * 4], edx
        inc ecx
        
        add edx, 4096
        cmp ecx, 1024
        je map_kernel
        jmp .l0    
    
map_kernel:
    mov edx, KERNEL_MAPPED
    mov ebx, edx
    shr ebx, 22
    mov ecx, edx
    shr ecx, 12
    and ecx, 0x3ff
    
    mov dword [eax + ebx * 4], (PAGE_TABLES + 8192) | 3
    mov esi, PAGE_TABLES + 8192

    mov edx, KERNEL_MOVE
    or edx, 0x803
    .l0:
        mov [esi + ecx * 4], edx
        
        inc ecx
        add edx, 4096
        cmp ecx, 1024
        je load_pd
        jmp .l0
load_pd:
    pop eax
    mov cr3, eax
    cli
    
    lgdt [basic_gdt_info]
    mov eax, cr0
    or eax, 0x8000_0001
    mov cr0, eax
    
    jmp 0x8:protected_mode
bits 32
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    pop eax
    push kernel_info_struct
    jmp KERNEL_MAPPED
jmp $
bits 16
DEFAULT_PG_DIR  EQU 0x10000
PAGE_TABLES     EQU 0x20000
PHYS_MEM_KBITM  EQU 0x100000

MMAP_PTR        EQU 0x9000
MMAP_MAGIC      EQU 0x534D4150
VBE_PTR         EQU 0x8c00
VBE_MODE_OFF    EQU 0x0000
VBE_MODE_SEG    EQU 0x3000
VBE_MODE_PTR    EQU 0x00030000
VBE_EDID_PTR    EQU VBE_PTR + 256
virtualized: db 0

gen_mmap:
    .first:
        xor ebx, ebx
    .loop:
        mov edx, MMAP_MAGIC
        mov eax, 0xe820
        mov ecx, 24
        mov dword [es:di + 20], 1
        int 0x15
        jc .ret
        mov edx, MMAP_MAGIC
    .check:
        jcxz .skip
        cmp cl, 20
        jle .next_ent
        test byte [es:di + 20], 1
        je .skip
    .next_ent:
        mov eax, [es:di + 8]
        or eax, [es:di + 12]
        jz .skip
        inc dword [kernel_info_struct.mem_map_count]
        add di, 24
    .skip:
        cmp ebx, 0
        jne .loop
    .ret:
        clc
        ret
    .err:
        mov si, strings.memerr
        call print.string
        jmp load_boot.hltlp
    jmp $

vbe_err:            db "[Error 0x10] BIOS VBE Unsupported! (Non-Fatal)", 0xa, 0xd, 0
vbe_mode_count:     dd 0

kernel_info_struct:
    .mem_map:       dd MMAP_PTR
    .mem_map_count: dd 0
    .VBE_info:      dd VBE_PTR
    .EDID_info:     dd VBE_EDID_PTR
    .mode_info:     dd VESA_mode_info
    .mode_count:    dd 0
    .vbe_x_res:     dw 0
    .vbe_y_res:     dw 0
    .mon_x_res:  dw 0
    .mon_y_res:  dw 0
    .vbe_mode_id:   dw 0
    .color_mode:    dw 0;already deprecated
    .vbe_bpp:       db 0
    .framebuffer:   dd 0
    .cpuid_ns:      db 0
    .ramdisk:       dd 0
    .ramdisk_sz:    dq 0
    .kernel_pbitm:  dd PHYS_MEM_KBITM
    .bitmap_sz:     dd (KERNEL_MOVE - PHYS_MEM_KBITM)

COLOR_MODE_16   EQU 1
COLOR_MODE_8B   EQU 2
COLOR_MODE_15B  EQU 3
COLOR_MODE_16B  EQU 4
COLOR_MODE_24B  EQU 5
COLOR_MODE_32B  EQU 6
COLOR_MODE_TXT  EQU 7

mon_x_res:  dd 0
mon_y_res:  dd 0
mode:       dw 0
gen_vbe_info:    
    mov ax, 0x4f00
    int 0x10
    cmp ax, 0x004f
    jne .err
    ;offset into struct is 14
    mov si, [es:di + 14]
    push si
    mov ds, [es:di + 16]
    xor ecx, ecx
    .count:
        mov bx, [ds:si];hopefully the BIOS doesn't decide to put it at like 0xabcd:fff0 or something...
        cmp bx, 0xffff
        je .end
        add si, 2
        inc ecx
        jmp .count
    .end:
    pop si;reset SI to point to the modes properly
    lea ebx, [es:di]
    mov [es:vbe_mode_count], ecx
    lea eax, [ds:si]
    
    push eax
    push ebx
    push ecx
    
    .get_edid:
        add di, 256
        mov ax, 0x4f15
        xor ebx, ebx
        mov bl, 1
        xor cx, cx
        xor edx, edx
        ;es:di already points to the buffer i want to store it at
        ;there may be a bug here on real hardware so watch out
        ;edid is VBE_INFO + 256
        int 0x10
        cmp ax, 0x004f
        jne .err
    .get_opt_size:
        pop ecx
        pop ebx
        pop eax
        add ebx, 256
        xor edx, edx
        
        push ecx
        mov cl, [ebx + 0x36 + 2]
        
        mov dl, [ebx + 0x36 + 4]
        and edx, 0xf0
        shl edx, 4
        or edx, ecx
        mov [mon_x_res], edx
        
        xor ecx, ecx
        xor edx, edx
        
        mov cl, [ebx + 0x36 + 5]
        mov dl, [ebx + 0x36 + 7]
        and edx, 0xf0
        shl edx, 4
        or edx, ecx
        mov [mon_y_res], edx
        
        pop ecx
        push eax
        mov eax, (VBE_MODE_PTR & 0xf0000) >> 4
        mov es, ax
        mov di, VBE_MODE_PTR & 0xffff
        pop ebx ;redundant? maybe. Stupid? sure. Wrong? probably
        mov ebx, VBE_PTR
        mov ds, [ebx + 16]
        mov si, [ebx + 14]
    .find_mode:
        ;vbe_mode_ID is the mode we've selected
        ;es:di is the staging ground for the mode so we can compare data
        ;ds:si is the ptr to the mode array
        ;ebx stores the remaining count
        ;ax and cx must be used for interrupts
        ;edx is free
        ;
        ; jmp $
        xor ax, ax
        mov es, ax
        mov di, VESA_mode_info
        mov cx, [ds:si]
        mov ax, 0x4f01
        int 0x10
        cmp ax, 0x004f
        jne .err
    .check_mode:
        xor edx, edx
        mov dx, word [VESA_mode_info.width] ;20 = width, 22 = height, 26 = bpp
        cmp dx, [mon_x_res]
        jg .next    ;if dx is greater than mon_x_res
        
        mov cx, word [VESA_mode_info.height]
        cmp cx, [mon_y_res]
        jg .next ;modes that are out of bounds will be skipped now
        mov al, [VESA_mode_info.bits_per_pixel]
        cmp dx, [kernel_info_struct.vbe_x_res]
        jl .next
        
        cmp cx, [kernel_info_struct.vbe_y_res]
        jl .next
        je .cmp_bpp
        jmp .set
        .cmp_bpp:
            cmp al, [kernel_info_struct.vbe_bpp]
            jl .next
    .set:
        push dx
        mov dx, [VESA_mode_info.attributes]
        and dx, 1<<7
        jz .next
        pop dx
        
        mov [kernel_info_struct.vbe_x_res], dx
        mov [kernel_info_struct.vbe_y_res], cx
        mov [kernel_info_struct.vbe_bpp], al
        mov eax, [VESA_mode_info.framebuffer]
        mov [kernel_info_struct.framebuffer], eax
        mov cx, [ds:si]
        mov [mode], cx
        mov [kernel_info_struct.vbe_mode_id], cx
        
    .next:
        add si, 2
        cmp word [ds:si], 0xffff
        je .end_select_mode
        jmp .find_mode
    .end_select_mode:
        mov cx, [mode]
        mov ax, 0x4f01
        int 0x10
        mov bx, cx
        mov ax, 0x4f02
        or bx, 0x4000
        int 0x10
        cmp ax, 0x004f
        jne .err
        mov dx, [mon_x_res]
        mov [kernel_info_struct.mon_x_res], dx
        mov dx, [mon_y_res]
        mov [kernel_info_struct.mon_y_res], dx
        ; mov dx, [VESA_mode_info.width]
        ; mov cx, [VESA_mode_info.height]
        ; xor bx, bx
        ; mov bl, [VESA_mode_info.bits_per_pixel]
        ; mov eax, [VESA_mode_info.framebuffer]
        ; mov edi, eax
        ; mov eax, 0x00ffffff
        ; mov [edi], eax
        ; jmp $ ;all of this was debug
        pop ax
        pop ax
        ret
    
    .err:
        mov ah, 0x0
        mov al, 0x3
        int 0x10
        mov si, vbe_err
        call print.string
        
        
        mov dword [kernel_info_struct.VBE_info], 0
        mov dword [kernel_info_struct.EDID_info], 0
        mov word [kernel_info_struct.vbe_mode_id], 5
        mov word [kernel_info_struct.color_mode], COLOR_MODE_TXT
        mov word [kernel_info_struct.vbe_x_res], 80
        mov word [kernel_info_struct.vbe_y_res], 25
        mov dword [kernel_info_struct.framebuffer], 0xb8000
        xor eax, eax
        mov ebx, eax
        mov ecx, eax
        mov edx, eax
        mov ds, ax
        mov es, ax
        
        ret
        

VESA_mode_info:
    .attributes:    dw 0
    .window_a:      db 0
    .window_b:      db 0
    .granularity:   dw 0
    .window_size:   dw 0
    .segment_a:     dw 0
    .segment_b:     dw 0
    .win_func_ptr:  dd 0
    .pitch:         dw 0    ;bytes per line
    .width:         dw 0
    .height:        dw 0
    .w_char:        db 0
    .y_char:        db 0
    .planes:        db 0
    .bits_per_pixel:db 0
    .banks:         db 0
    .mem_model:     db 0
    .bank_size:     db 0
    .image_pages:   db 0
    .reserved0:     db 0
    .red_mask:      db 0
    .red_position:  db 0
    .green_mask:    db 0
    .green_position:db 0
    .blue_mask:     db 0
    .blue_position: db 0
    .reserved_mask: db 0
    .reserved_pos:  db 0
    .direct_color:  db 0
    .framebuffer:   dd 0
    .off_screen_mem:dd 0
    .off_screen_sz: dw 0
    .reserved1: times 206 db 0
    
basic_gdt_info:
    .size: dw basic_gdt.end - basic_gdt - 1
    .offset: dd basic_gdt
basic_gdt:
    .NULL:
        dd 0
        dd 0
    .CODE:
        dw 0xffff
        dw 0
        db 0
        db 0b10011010
        db 0b11001111
        db 0
    .DATA:
        dw 0xffff
        dw 0
        db 0
        db 0b10010010
        db 0b11001111
        db 0
    .end:
times 2048 - ($-$$) db 0
times 4096 - ($-$$) db 0