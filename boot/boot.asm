; MatrixOS Bootloader
; Version 1.7 - Single Sector LBA Test

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

    ; A = bootloader started
    mov al, 'A'
    call print_char

    ; Check BIOS EDD
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]

    int 0x13
    jc error

    cmp bx, 0xAA55
    jne error

    test cx, 1
    jz error

    ; B = EDD works
    mov al, 'B'
    call print_char

    ; Read ONLY ONE sector
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42

    int 0x13
    jc error

    ; C = one-sector read works
    mov al, 'C'
    call print_char

    cli
    hlt

error:
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

print_char:
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    ret


align 4

dap:
    db 0x10
    db 0x00

    ; ONE sector
    dw 1

    ; Destination 0000:1000
    dw 0x1000
    dw 0x0000

    ; LBA 1
    dq 1


boot_drive:
    db 0

error_message:
    db "MatrixOS: Disk error", 0


times 510 - ($ - $$) db 0

dw 0xAA55
