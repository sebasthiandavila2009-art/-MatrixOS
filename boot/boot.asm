; MatrixOS Bootloader
; Version 5.1 - Complete Floppy Kernel Loader

BITS 16
ORG 0x7C00

start:
    cli

    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; ========================================
    ; Load MatrixOS kernel
    ;
    ; Current kernel = 10408 bytes
    ; Required = 21 sectors
    ;
    ; Head 0: sectors 2-18 = 17 sectors
    ; Head 1: sectors 1-4  = 4 sectors
    ;
    ; Kernel destination = 0000:1000
    ; ========================================

    xor ax, ax
    mov es, ax

    mov bx, 0x1000

    ; ----------------------------------------
    ; First 17 sectors — head 0
    ; ----------------------------------------

    mov ah, 0x02
    mov al, 17
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    add bx, 8704

    ; ----------------------------------------
    ; Remaining 4 sectors — head 1
    ; ----------------------------------------

    mov ah, 0x02
    mov al, 12
    mov ch, 0
    mov cl, 1
    mov dh, 1
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    ; ========================================
    ; VGA Mode 13h
    ; ========================================

    mov ax, 0x0013
    int 0x10

    ; ========================================
    ; Load GDT
    ; ========================================

    cli
    lgdt [gdt_descriptor]

    ; ========================================
    ; Enable protected mode
    ; ========================================

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; ========================================
    ; Far jump to protected mode
    ; ========================================

    db 0x66
    db 0xEA
    dd protected_mode
    dw CODE_SELECTOR


; ========================================
; Disk Error
; ========================================

disk_error:
    mov si, msg_error

.print:
    lodsb
    test al, al
    jz .hang

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp .print

.hang:
    cli
    hlt
    jmp .hang


; ========================================
; Protected Mode
; ========================================

BITS 32

protected_mode:

    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    cld

    mov eax, 0x1000
    jmp eax


; ========================================
; GDT
; ========================================

BITS 16

gdt_start:

    dq 0

    ; Code
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Data
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
; Constants
; ========================================

CODE_SELECTOR equ 0x08
DATA_SELECTOR equ 0x10


; ========================================
; Variables
; ========================================

boot_drive:
    db 0


; ========================================
; Messages
; ========================================

msg_error:
    db 'X', 0


; ========================================
; Boot Signature
; ========================================

times 510-($-$$) db 0
dw 0xAA55
