; ========================================
; MatrixOS Bootloader
; Version 2.9 - Boot Diagnostic
; ========================================

BITS 16
ORG 0x7C00

start:

    cli

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; ----------------------------
    ; A = bootloader started
    ; ----------------------------

    mov al, 'A'
    call print_char

    ; ----------------------------
    ; Reset disk
    ; ----------------------------

    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; ----------------------------
    ; B = disk reset worked
    ; ----------------------------

    mov al, 'B'
    call print_char

    ; ----------------------------
    ; Load kernel
    ; ----------------------------

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

    ; ----------------------------
    ; C = kernel completely loaded
    ; ----------------------------

    mov al, 'C'
    call print_char

    ; ----------------------------
    ; D = bootloader continues
    ; ----------------------------

    mov al, 'D'
    call print_char

    ; ----------------------------
    ; E = bootloader finished
    ; ----------------------------

    mov al, 'E'
    call print_char

halt:

    cli
    hlt
    jmp halt


; ========================================
; BIOS Text Output
; ========================================

print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    ret


; ========================================
; Disk Error
; ========================================

disk_error:

    mov si, error_message

error_loop:

    lodsb

    test al, al
    jz halt

    call print_char
    jmp error_loop


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
