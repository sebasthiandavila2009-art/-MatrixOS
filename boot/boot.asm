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

    mov si, msg1
    call print_string

    ; Load kernel
    mov ah, 0x02
    mov al, 0x02
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13
    jc disk_error

    mov si, msg2
    call print_string

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32-bit protected-mode far jump
    db 0x66
    db 0xEA
    dd protected_mode
    dw 0x08


print_string:
    lodsb
    test al, al
    jz .done

    mov ah, 0x0E
    int 0x10
    jmp print_string

.done:
    ret


disk_error:
    mov si, msg_error
    call print_string

hang:
    cli
    hlt
    jmp hang


BITS 32

protected_mode:

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov esp, 0x90000

    ; P = protected mode reached
    mov eax, 0x07500050
    mov [0xB8000], eax

    ; Kernel
    jmp 0x08:0x1000


gdt_start:

    dq 0

code_descriptor:
    dw 0xFFFF
    dw 0
    db 0
    db 0x9A
    db 0xCF
    db 0

data_descriptor:
    dw 0xFFFF
    dw 0
    db 0
    db 0x92
    db 0xCF
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start


boot_drive:
    db 0

msg1:
    db "MATRIXOS: Bootloader OK", 13, 10, 0

msg2:
    db "MATRIXOS: Kernel loaded", 13, 10, 0

msg_error:
    db "MATRIXOS: Disk error", 13, 10, 0


times 510 - ($ - $$) db 0
dw 0xAA55
