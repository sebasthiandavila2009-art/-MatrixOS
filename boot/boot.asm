; MatrixOS Bootloader
; Version 1.1 - 30 Sector Kernel Loader

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

    ; Enable A20
    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ; Check BIOS LBA support
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    cmp bx, 0xAA55
    jne disk_error

    ; ---------------------------------------------------------
    ; Load MatrixOS kernel
    ;
    ; Start LBA: 1
    ; Sectors: 30
    ; Destination: 0x1000
    ;
    ; 30 x 512 = 15,360 bytes
    ; ---------------------------------------------------------

    mov si, disk_address_packet

    mov ah, 0x42
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    ; Enter protected mode
    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    jmp 0x08:protected_mode


disk_error:

    mov si, error_message

.print:

    lodsb

    cmp al, 0
    je .hang

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp .print

.hang:

    cli
    hlt
    jmp .hang


BITS 32

protected_mode:

    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    jmp 0x1000


; =============================================================
; GDT
; =============================================================

gdt_start:

gdt_null:
    dq 0x0000000000000000

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


; =============================================================
; BIOS Extended Disk Address Packet
; =============================================================

disk_address_packet:

    db 0x10
    db 0x00

    ; 30 sectors x 512 bytes
    dw 30 * 512

    ; Destination
    dw 0x1000
    dw 0x0000

    ; Start at sector/LBA 1
    dq 1


; =============================================================
; Variables
; =============================================================

boot_drive:

    db 0


error_message:

    db "MatrixOS: Disk error", 0


; =============================================================
; Boot Signature
; =============================================================

times 510 - ($ - $$) db 0

dw 0xAA55
