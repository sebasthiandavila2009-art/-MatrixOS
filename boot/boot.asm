; MatrixOS Bootloader
; Version 1.6 - Boot Diagnostic

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

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Check LBA support
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc error

    cmp bx, 0xAA55
    jne error

    test cx, 1
    jz error

    ; B = LBA support works
    mov al, 'B'
    call print_char

    ; Read kernel
    mov si, dap1
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc retry

    ; C = first read works
    mov al, 'C'
    call print_char

    ; Read remaining kernel space
    mov si, dap2
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc retry2

    ; D = second read works
    mov al, 'D'
    call print_char

    ; Enter protected mode
    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; E = protected mode entry
    jmp 0x08:protected_mode


retry:
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    mov si, dap1
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc error

    mov al, 'C'
    call print_char

    mov si, dap2
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc error

    mov al, 'D'
    call print_char

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


retry2:
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    mov si, dap2
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc error

    mov al, 'D'
    call print_char

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


error:
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


align 4

dap1:
    db 0x10
    db 0
    dw 16
    dw 0x1000
    dw 0
    dq 1


align 4

dap2:
    db 0x10
    db 0
    dw 14
    dw 0x3000
    dw 0
    dq 17


boot_drive:
    db 0

error_message:
    db "MatrixOS: Disk error", 0


times 510 - ($ - $$) db 0

dw 0xAA55
