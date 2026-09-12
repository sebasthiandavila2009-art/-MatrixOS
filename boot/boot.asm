; MatrixOS Bootloader
; Version 1.5 - Reliable 30 Sector LBA Loader

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

    ; Check BIOS Extended Disk Services
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    cmp bx, 0xAA55
    jne disk_error

    test cx, 1
    jz disk_error

    ; -----------------------------------------
    ; READ 1
    ; 16 sectors
    ; LBA 1
    ; Destination: 0000:1000
    ; -----------------------------------------

    mov si, dap1
    mov dl, [boot_drive]
    mov ah, 0x42

    int 0x13
    jnc read2

    ; Reset and retry
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    mov si, dap1
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error


read2:

    ; -----------------------------------------
    ; READ 2
    ; 14 sectors
    ; LBA 17
    ; Destination: 0000:3000
    ; -----------------------------------------

    mov si, dap2
    mov dl, [boot_drive]
    mov ah, 0x42

    int 0x13
    jnc protected

    ; Reset and retry
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    mov si, dap2
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jc disk_error


protected:

    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


disk_error:

    mov si, error_message

print_error:

    lodsb

    test al, al
    jz halt

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp print_error


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

; -----------------------------------------
; GDT
; -----------------------------------------

gdt_start:

    dq 0

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


; -----------------------------------------
; DAP 1
; 16 sectors
; LBA 1
; 0000:1000
; -----------------------------------------

align 4

dap1:

    db 0x10
    db 0x00

    dw 16

    dw 0x1000
    dw 0x0000

    dq 1


; -----------------------------------------
; DAP 2
; 14 sectors
; LBA 17
; 0000:3000
; -----------------------------------------

align 4

dap2:

    db 0x10
    db 0x00

    dw 14

    dw 0x3000
    dw 0x0000

    dq 17


boot_drive:

    db 0


error_message:

    db "MatrixOS: Disk error", 0


times 510 - ($ - $$) db 0

dw 0xAA55
