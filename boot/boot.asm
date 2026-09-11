; MatrixOS Bootloader
; Version 0.7

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

    ; Bootloader message
    mov si, boot_message
    call print_string

    ; Load kernel: 2 sectors -> physical address 0x1000
    mov ah, 0x02
    mov al, 2
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    mov bx, 0x1000
    int 0x13

    jc disk_error

    ; Kernel loaded
    mov si, kernel_message
    call print_string

    ; Enable A20
    in al, 0x92
    or al, 00000010b
    out 0x92, al

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32-bit far jump
    db 0x66
    db 0xEA
    dd protected_mode
    dw 0x08


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

.hang:
    cli
    hlt
    jmp .hang


; =========================================
; 32-BIT PROTECTED MODE
; =========================================

BITS 32

protected_mode:

    ; Reload data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Set 32-bit stack
    mov esp, 0x90000

    ; Write a visible protected-mode message
    mov edi, 0xB8000

    mov ax, 0x074D
    mov [edi], ax

    mov ax, 0x0752
    mov [edi + 2], ax

    mov ax, 0x0749
    mov [edi + 4], ax

    mov ax, 0x0744
    mov [edi + 6], ax

    mov ax, 0x0745
    mov [edi + 8], ax

    ; Stay here for now
.hang:
    cli
    hlt
    jmp .hang


; =========================================
; DATA
; =========================================

boot_drive db 0

boot_message:
    db 'MATRIXOS: Bootloader OK', 13, 10, 0

kernel_message:
    db 'MATRIXOS: Kernel loaded', 13, 10, 0

error_message:
    db 'MATRIXOS: Disk error', 13, 10, 0


; =========================================
; GDT
; =========================================

gdt_start:

    ; Null descriptor
    dq 0

    ; Code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Data segment
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


; Boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
