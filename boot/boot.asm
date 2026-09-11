; MatrixOS Bootloader
; Version 2.1 - Protected Mode Diagnostic

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

    mov [boot_drive], dl

    mov si, boot_message
    call print_string

    ; Load kernel: sectors 2-3 -> 0x1000
    mov ah, 0x02
    mov al, 0x02
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    mov bx, 0x1000

    int 0x13
    jc disk_error

    mov si, kernel_message
    call print_string

    ; A20
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; GDT
    lgdt [gdt_descriptor]

    ; REAL-MODE CHECKPOINT
    ; R = reached protected-mode setup
    mov word [0xB8000], 0x0752

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; FAR JUMP
    db 0xEA
    dw protected_mode
    dw 0x0008


print_string:
    lodsb
    test al, al
    jz .done

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp print_string

.done:
    ret


disk_error:
    mov si, error_message
    call print_string

.hang:
    cli
    hlt
    jmp .hang


BITS 32

protected_mode:

    ; Protected-mode data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    ; PROTECTED-MODE CHECKPOINT
    ; P = successfully entered protected mode
    mov word [0xB8000], 0x0750

    ; Jump to kernel
    jmp 0x08:0x1000


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


boot_drive:
    db 0


boot_message:
    db "MATRIXOS: Bootloader OK", 13, 10, 0

kernel_message:
    db "MATRIXOS: Kernel loaded", 13, 10, 0

error_message:
    db "MATRIXOS: Disk error", 13, 10, 0


times 510 - ($ - $$) db 0
dw 0xAA55
