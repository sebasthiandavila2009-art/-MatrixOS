; ========================================
; MatrixOS Bootloader
; Version 3.3 - Stable Protected Mode
; ========================================

BITS 16
ORG 0x7C00

CODE16_SELECTOR equ 0x08
CODE32_SELECTOR equ 0x10
DATA_SELECTOR   equ 0x18

start:

    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; ====================================
    ; A = bootloader started
    ; ====================================

    mov al, 'A'
    call print_char

    ; ====================================
    ; Reset disk
    ; ====================================

    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; B = disk reset worked
    mov al, 'B'
    call print_char

    ; ====================================
    ; Load kernel
    ; ====================================

    xor ax, ax
    mov es, ax
    mov bx, 0x1000

    ; Kernel = 8004 bytes
    ; 16 sectors
    ;
    ; Boot sector = sector 1
    ; Kernel      = sectors 2-17

    mov byte [sector], 2
    mov byte [sectors_left], 16

load_kernel:

    mov ah, 0x02
    mov al, 1

    mov ch, 0
    mov cl, [sector]

    mov dh, 0
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    add bx, 512

    inc byte [sector]

    dec byte [sectors_left]
    jnz load_kernel

    ; C = kernel successfully loaded

    mov al, 'C'
    call print_char

    ; ====================================
    ; Load GDT
    ; ====================================

    cli

    lgdt [gdt_descriptor]

    ; ====================================
    ; Enable protected mode
    ; ====================================

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; ====================================
    ; Enter 16-bit protected mode first
    ; ====================================

    jmp CODE16_SELECTOR:protected_mode16


; ========================================
; BIOS text output
; ========================================

print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    ret


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
; 16-bit Protected Mode
; ========================================

BITS 16

protected_mode16:

    ; ====================================
    ; Load valid data segment
    ; ====================================

    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Protected-mode stack

    mov sp, 0x9000

    ; ====================================
    ; P = protected mode reached
    ; ====================================

    mov word [0xB8000], 0x0F50

    ; ====================================
    ; Switch to 32-bit protected mode
    ; ====================================

    jmp CODE32_SELECTOR:protected_mode32


; ========================================
; 32-bit Protected Mode
; ========================================

BITS 32

protected_mode32:

    ; ====================================
    ; Load data segments again
    ; ====================================

    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 32-bit stack

    mov esp, 0x90000

    ; ====================================
    ; M = 32-bit protected mode working
    ; ====================================

    mov word [0xB8002], 0x0F4D

    ; ====================================
    ; K = about to start kernel
    ; ====================================

    mov word [0xB8004], 0x0F4B

    ; ====================================
    ; Jump to MatrixOS kernel
    ; ====================================

    mov eax, 0x1000
    jmp eax


; ========================================
; Global Descriptor Table
; ========================================

BITS 16

gdt_start:

    ; ------------------------------------
    ; Null descriptor
    ; ------------------------------------

    dq 0


    ; ------------------------------------
    ; 16-bit code segment
    ; Selector = 0x08
    ; ------------------------------------

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 00001111b
    db 0x00


    ; ------------------------------------
    ; 32-bit code segment
    ; Selector = 0x10
    ; ------------------------------------

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00


    ; ------------------------------------
    ; 32-bit data segment
    ; Selector = 0x18
    ; ------------------------------------

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

sector:
    db 2

sectors_left:
    db 16

error_message:
    db "MatrixOS: Disk error", 0


; ========================================
; Boot signature
; ========================================

times 510 - ($ - $$) db 0

dw 0xAA55
