; ========================================
; MatrixOS Bootloader
; Version 2.8 - Simple CHS Boot
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

    ; Show A
    mov al, 'A'
    call print_char

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Show B
    mov al, 'B'
    call print_char

    ; Load kernel
    ; Kernel size: 8004 bytes
    ; 16 sectors are required.
    ;
    ; Sector 1 = bootloader
    ; Sectors 2-17 = kernel

    xor ax, ax
    mov es, ax
    mov bx, 0x1000

    mov byte [sector], 2
    mov byte [sectors_left], 16

load_loop:

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
    jnz load_loop

    ; Show C
    mov al, 'C'
    call print_char

    ; ====================================
    ; Protected mode
    ; ====================================

    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump into 32-bit code
    jmp CODE_SELECTOR:protected_mode


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
; Protected Mode
; ========================================

BITS 32

protected_mode:

    ; Show D
    mov word [0xB8000], 0x0F44

    ; Data segment
    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Stack
    mov esp, 0x90000

    ; Show E
    mov word [0xB8002], 0x0F45

    ; Jump to kernel
    mov eax, 0x1000
    jmp eax


; ========================================
; GDT
; ========================================

BITS 16

gdt_start:

    dq 0


gdt_code:

    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00


gdt_data:

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
