; MatrixOS Bootloader
; Version 0.6

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

    ; Boot message
    mov si, boot_message
    call print_string

    ; Load kernel from sectors 2 and 3
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

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Explicit 32-bit far jump
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
; PROTECTED MODE
; =========================================

BITS 32

protected_mode:

    ; Load data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Protected-mode stack
    mov esp, 0x90000

    ; Diagnostic: MRIDE
    mov word [0xB8000], 0x074D
    mov word [0xB8002], 0x0752
    mov word [0xB8004], 0x0749
    mov word [0xB8006], 0x0744
    mov word [0xB8008], 0x0745

    ; Jump to kernel
    db 0xEA
    dd 0x1000
    dw 0x08


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


; Boot signature
times 510 - ($ - $$) db 0
dw 0xAA55
