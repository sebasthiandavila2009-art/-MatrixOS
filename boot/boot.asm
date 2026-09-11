; MatrixOS Bootloader
; Version 3.2 - 12 Sector Kernel

BITS 16
ORG 0x7C00

start:
    cli
    cld

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Save BIOS boot drive
    mov [boot_drive], dl

    ; Boot message
    mov si, boot_message
    call print_string

    ; --------------------------------
    ; Load MatrixOS kernel
    ; 12 sectors
    ; Kernel starts at sector 2
    ; Loaded to physical address 0x1000
    ; --------------------------------

    mov ah, 0x02
    mov al, 12
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000

    int 0x13
    jc disk_error

    mov si, kernel_message
    call print_string

    ; --------------------------------
    ; Enable VGA Mode 13h
    ; --------------------------------

    mov ax, 0x0013
    int 0x10

    ; --------------------------------
    ; Load GDT
    ; --------------------------------

    lgdt [gdt_descriptor]

    ; --------------------------------
    ; Enable protected mode
    ; --------------------------------

    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Jump to 32-bit protected mode
    jmp 0x08:protected_mode


; ================================================
; BIOS Text Output
; ================================================

print_string:
    lodsb

    test al, al
    jz .done

    mov ah, 0x0E
    mov bh, 0x00
    int 0x10

    jmp print_string

.done:
    ret


; ================================================
; Disk Error
; ================================================

disk_error:
    mov si, error_message
    call print_string

.hang:
    cli
    hlt
    jmp .hang


; ================================================
; 32-bit Protected Mode
; ================================================

BITS 32

protected_mode:

    mov ax, 0x10

    mov ds, ax
    mov es
