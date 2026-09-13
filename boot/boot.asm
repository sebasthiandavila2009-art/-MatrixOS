; ========================================
; MatrixOS Bootloader
; Version 2.6 - Clean Protected Mode
; ========================================

BITS 16
ORG 0x7C00

CODE_SELECTOR equ 0x08
DATA_SELECTOR equ 0x10

start:

    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; A = bootloader started
    mov al, 'A'
    call print_char

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; B = disk reset works
    mov al, 'B'
    call print_char

    ; Get disk geometry
    mov ah, 0x08
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    mov al, cl
    and al, 0x3F
    mov [sectors_per_track], al

    mov [max_head], dh

    ; Load kernel at physical address 0x1000
    xor ax, ax
    mov es, ax
    mov bx, 0x1000

    mov byte [current_sector], 2
    mov byte [current_head], 0
    mov word [current_cylinder], 0

    ; Load 30 sectors
    mov byte [sectors_left], 30


load_sector:

    mov ah, 0x02
    mov al, 1

    mov ch, byte [current_cylinder]
    mov cl, byte [current_sector]
    mov dh, byte [current_head]
    mov dl, [boot_drive]

    int 0x13
    jc retry_read

    add bx, 512

    dec byte [sectors_left]
    jz kernel_loaded

    inc byte [current_sector]

    mov al, [sectors_per_track]

    cmp byte [current_sector], al
    jbe load_sector

    mov byte [current_sector], 1

    inc byte [current_head]

    mov al, [max_head]

    cmp byte [current_head], al
    jbe load_sector

    mov byte [current_head], 0

    inc word [current_cylinder]

    jmp load_sector


retry_read:

    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    mov ah, 0x02
    mov al, 1

    mov ch, byte [current_cylinder]
    mov cl, byte [current_sector]
    mov dh, byte [current_head]
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    add bx, 512

    dec byte [sectors_left]
    jz kernel_loaded

    inc byte [current_sector]

    mov al, [sectors_per_track]

    cmp byte [current_sector], al
    jbe load_sector

    mov byte [current_sector], 1

    inc byte [current_head]

    mov al, [max_head]

    cmp byte [current_head], al
    jbe load_sector

    mov byte [current_head], 0

    inc word [current_cylinder]

    jmp load_sector


kernel_loaded:

    ; C = kernel loaded
    mov al, 'C'
    call print_char

    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump into protected mode
    jmp CODE_SELECTOR:protected_mode


; ========================================
; Disk error
; ========================================

disk_error:

    mov si, error_message

error_loop:

    lodsb

    test al, al
    jz halt

    call print_char
    jmp error_loop


halt:

    cli
    hlt
    jmp halt


; ========================================
; BIOS text output
; ========================================

print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    ret


; ========================================
; Protected Mode
; ========================================

BITS 32

protected_mode:

    ; D = protected mode reached
    mov word [0xB8000], 0x0F44

    ; Load data selector
    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Protected-mode stack
    mov esp, 0x90000

    ; E = kernel jump
    mov word [0xB8002], 0x0F45

    ; Jump to kernel
    jmp 0x1000


; ========================================
; Global Descriptor Table
; ========================================

BITS 16

gdt_start:

    ; Null descriptor
    dq 0


gdt_code:

    ; 32-bit code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00


gdt_data:

    ; 32-bit data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00


gdt_end:


gdt_descriptor:

    dw gdt_end - gdt_start - 1
    dd gdt_start


; ========================================
; Variables
; ========================================

boot_drive:
    db 0

sectors_per_track:
    db 18

max_head:
    db 1

current_sector:
    db 2

current_head:
    db 0

current_cylinder:
    dw 0

sectors_left:
    db 30

error_message:
    db "MatrixOS: Disk error", 0


; ========================================
; Boot signature
; ========================================

times 510 - ($ - $$) db 0

dw 0xAA55
