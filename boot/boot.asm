; MatrixOS Bootloader
; Version 0.2 Debug

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

    ; Debug: bootloader is running
    mov si, boot_message
    call print_string

    ; Load kernel from sector 2
    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13

    jc disk_error

    ; Debug: kernel loaded
    mov si, kernel_message
    call print_string

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode

print_string:
.next:
    lodsb
    test al, al
    jz .done

    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .next

.done:
    ret

disk_error:
    mov si, error_message
    call print_string

.halt:
    cli
    hlt
    jmp .halt

BITS 32

protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov esp, 0x90000

    ; Jump to C kernel
    jmp 0x08:0x1000

boot_drive db 0

boot_message:
    db 'MATRIXOS: Bootloader OK', 13, 10, 0

kernel_message:
    db 'MATRIXOS: Kernel loaded', 13, 10, 0

error_message:
    db 'MATRIXOS: Disk error', 13, 10, 0

gdt_start:

gdt_null:
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

times 510 - ($ - $$) db 0
dw 0xAA55
