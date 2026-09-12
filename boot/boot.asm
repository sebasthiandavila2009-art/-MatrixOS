; MatrixOS Bootloader
; Version 1.9 - BIOS Diagnostic

BITS 16
ORG 0x7C00

start:

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

    ; Read ONE sector only
    mov ax, 0x1000
    mov es, ax

    xor bx, bx

    mov ah, 0x02
    mov al, 1

    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    ; Show C
    mov al, 'C'
    call print_char

    cli

    ; Protected mode
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


disk_error:

    mov si, error_message

error_loop:

    lodsb

    test al, al
    jz halt

    call print_char

    jmp error_loop


print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    ret


halt:

    cli
    hlt
    jmp halt


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


BITS 16

gdt_start:

    dq 0

gdt_code:

    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:

    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:

    dw gdt_end - gdt_start - 1
    dd gdt_start


boot_drive:

    db 0


error_message:

    db "MatrixOS: Disk error", 0


times 510 - ($ - $$) db 0

dw 0xAA55
