; MatrixOS Bootloader
; Version 4.0 - Clean Protected Mode Transition

BITS 16
ORG 0x7C00

start:

    cli

    ; Save BIOS boot drive
    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; A
    mov si, msg_a
    call print_string

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; B
    mov si, msg_b
    call print_string

    ; ------------------------------------
    ; Load kernel
    ; Sectors 2-17
    ; Address 0x1000
    ; ------------------------------------

    xor ax, ax
    mov es, ax

    mov bx, 0x1000
    mov byte [current_sector], 2
    mov byte [sectors_left], 16

load_kernel:

    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, [current_sector]
    mov dh, 0
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    add bx, 512

    inc byte [current_sector]
    dec byte [sectors_left]

    jnz load_kernel

    ; C
    mov si, msg_c
    call print_string

    ; ------------------------------------
    ; Load GDT
    ; ------------------------------------

    cli
    lgdt [gdt_descriptor]

    ; ------------------------------------
    ; Enable protected mode
    ; ------------------------------------

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; ------------------------------------
    ; 32-bit far jump
    ;
    ; protected_mode already contains
    ; the ORG-adjusted address 0x7Cxx.
    ; DO NOT add 0x7C00 again.
    ; ------------------------------------

    db 0x66
    db 0xEA
    dd protected_mode
    dw CODE_SELECTOR


; ========================================
; BIOS text output
; ========================================

print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10
    ret


print_string:

.next:
    lodsb
    test al, al
    jz .done

    call print_char
    jmp .next

.done:
    ret


; ========================================
; Disk error
; ========================================

disk_error:

    mov si, msg_error
    call print_string

.hang:
    cli
    hlt
    jmp .hang


; ========================================
; Protected Mode
; ========================================

BITS 32

protected_mode:

    ; Set data segments FIRST
    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Protected-mode stack
    mov esp, 0x90000

    cld

    ; D
    mov word [0xB8000], 0x0F44

    ; E
    mov word [0xB8002], 0x0F45

    ; Jump to kernel
    mov eax, 0x1000
    jmp eax


; ========================================
; GDT
; ========================================

BITS 16

gdt_start:

    ; Null
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

current_sector:
    db 2

sectors_left:
    db 16


; ========================================
; Messages
; ========================================

msg_a:
    db 'A', 0

msg_b:
    db 'B', 0

msg_c:
    db 'C', 0

msg_error:
    db 'X', 0


; ========================================
; Boot signature
; ========================================

times 510-($-$$) db 0

dw 0xAA55
