; MatrixOS Kernel Entry
; Version 0.5

BITS 32

global _start
extern kernel_main

_start:

    ; Set up stack
    mov esp, 0x90000

    ; Write directly to VGA memory
    mov word [0xB8000], 0x074D
    mov word [0xB8002], 0x0741
    mov word [0xB8004], 0x0754
    mov word [0xB8006], 0x0752
    mov word [0xB8008], 0x0749
    mov word [0xB800A], 0x0758
    mov word [0xB800C], 0x004F
    mov word [0xB800E], 0x0753
    mov word [0xB8010], 0x004B

    ; Call C kernel
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
