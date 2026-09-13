; ========================================
; MatrixOS Bootloader
; Version 3.1 - Stable Protected Mode
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

    ; A
    mov al, 'A'
    call print_char

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; B
    mov al, 'B'
    call print_char

    ; --------------------------------
    ; Load kernel
    ; --------------------------------

    xor ax, ax
    mov es, ax
    mov bx, 0x1000

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

    ; C
    mov al, 'C'
    call print_char

    ; --------------------------------
    ; Load GDT
    ; --------------------------------

    lgdt [gdt_descriptor]

    ; D
    mov al, 'D'
    call print_char

    ; --------------------------------
    ; Enable protected mode
    ; --------------------------------

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; --------------------------------
    ; Switch to protected mode
    ;
    ; The destination is below 64K,
    ; so a 16-bit offset is sufficient.
    ; The code segment itself is 32-bit.
    ; --------------------------------

    jmp CODE_SELECTOR:protected_mode


; ========================================
; BIOS output
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
; Protected Mode
; ========================================

BITS 32

protected_mode:

    ; P = Protected Mode reached
    mov word [0xB8000], 0x0F50

    ; Data segment
    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Protected-mode stack
    mov esp, 0x90000

    ; M = protected mode working
    mov word [0xB8002], 0x0F4D

    cli

protected_hang:

    hlt
    jmp protected_hang


; ========================================
; Global Descriptor Table
; ========================================

BITS 16

gdt_start:

    ; Null descriptor
    dq 0

    ; Code descriptor
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Data descriptor
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
; Boot Signature
; ========================================

times 510 - ($ - $$) db 0

dw 0xAA55
