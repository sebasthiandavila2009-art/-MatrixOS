; MatrixOS Bootloader
; Version 1.8 - Stable Protected Mode Transition

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

    ; Load kernel from sectors 2-3
    ; Physical address: 0x1000
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

    ; Enable A20
    in al, 0x92
    or al, 0x02
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Far jump into 32-bit protected mode
    jmp 0x08:protected_mode


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


disk_error:
    mov si, error_message
    call print_string

.hang:
    cli
    hlt
    jmp .hang


BITS 32

protected_mode:

    ; Load protected-mode data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set stack
    mov esp, 0x90000

    ; Visible checkpoint
    mov word [0xB8000], 0x0750

    ; Jump directly to kernel entry
    jmp 0x08:0x1000


gdt_start:

gdt_null:
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
