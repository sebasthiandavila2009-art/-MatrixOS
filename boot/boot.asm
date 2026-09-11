; MatrixOS Bootloader
; Version 3.0 - 10 Sector Kernel

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
    ; 10 sectors -> physical 0x1000
    ; --------------------------------

    mov ah, 0x02
    mov al, 0x0B
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
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

    ; Jump to protected mode
    jmp 0x08:protected_mode


; --------------------------------
; BIOS text output
; --------------------------------

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


; --------------------------------
; Disk error
; --------------------------------

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
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    ; Jump to MatrixOS kernel
    jmp 0x08:0x1000


; ================================================
; Global Descriptor Table
; ================================================

gdt_start:

    dq 0

gdt_code:

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x9A
    db 0xCF
    db 0x00

gdt_data:

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


; ================================================
; Variables / Messages
; ================================================

boot_drive:
    db 0

boot_message:
    db "MATRIXOS: Bootloader OK", 13, 10, 0

kernel_message:
    db "MATRIXOS: Kernel loaded", 13, 10, 0

error_message:
    db "MATRIXOS: Disk error", 13, 10, 0


; Boot sector must be exactly 512 bytes
times 510 - ($ - $$) db 0

dw 0xAA55
