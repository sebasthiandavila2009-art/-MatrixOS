; MatrixOS Bootloader
; Version 0.1

BITS 16
ORG 0x7C00

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, message

.print:
    lodsb
    test al, al
    jz .halt

    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    jmp .print

.halt:
    cli

.wait:
    hlt
    jmp .wait

message db 'MATRIXOS v0.1', 13, 10
        db 'Bootloader started.', 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55
