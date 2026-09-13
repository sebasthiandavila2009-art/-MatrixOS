; MatrixOS Bootloader
; Version 3.7 - Clean 32-bit Protected Mode Transition

BITS 16
ORG 0x7C00

start:

    cli

    ; Save boot drive
    mov [boot_drive], dl

    ; Set up real-mode segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; ====================================
    ; Diagnostic A - bootloader started
    ; ====================================

    mov si, msg_a
    call print_string

    ; ====================================
    ; Reset disk
    ; ====================================

    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    jc disk_error

    ; ====================================
    ; Diagnostic B - disk reset worked
    ; ====================================

    mov si, msg_b
    call print_string

    ; ====================================
    ; Load MatrixOS kernel
    ;
    ; Kernel begins at sector 2.
    ; Load 16 sectors to physical 0x1000.
    ; ====================================

    mov bx, 0x1000
    mov byte [current_sector], 2
    mov byte [sectors_left], 16

load_kernel:

    mov ah, 0x02
    mov al, 1

    mov ch, 0
    mov cl, [current_sector]

    mov dh, 0
    mov dl, [boot_drive]

    mov es, ax

    ; ES must be zero.
    xor ax, ax
    mov es, ax

    mov bx, 0x1000

    int 0x13

    jc disk_error

    inc byte [current_sector]
    dec byte [sectors_left]

    jnz load_kernel

    ; ====================================
    ; Diagnostic C - kernel loaded
    ; ====================================

    mov si, msg_c
    call print_string

    ; ====================================
    ; Load GDT
    ; ====================================

    cli

    lgdt [gdt_descriptor]

    ; ====================================
    ; Enable protected mode
    ; ====================================

    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    ; ====================================
    ; Far jump into 32-bit protected mode
    ; ====================================

    jmp CODE_SELECTOR:protected_mode


; ========================================
; BIOS text output
; ========================================

print_char:

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    ret


print_string:

.next:

    lodsb

    test al, al
    jz .done

    call print_char

    jmp .next

.done:

    ret


; ========================================
; Disk error
; ========================================

disk_error:

    mov si, msg_error
    call print_string

.hang:

    cli
    hlt
    jmp .hang


; ========================================
; 32-bit Protected Mode
; ========================================

BITS 32

protected_mode:

    ; ====================================
    ; Load protected-mode data segments
    ; ====================================

    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; ====================================
    ; Set protected-mode stack
    ; ====================================

    mov esp, 0x90000

    cld

    ; ====================================
    ; Diagnostic D
    ; ====================================

    mov word [0xB8000], 0x0F44

    ; ====================================
    ; Diagnostic E
    ; ====================================

    mov word [0xB8002], 0x0F45

    ; ====================================
    ; Jump to MatrixOS kernel
    ; ====================================

    jmp 0x1000


; ========================================
; Global Descriptor Table
; ========================================

gdt_start:

    ; Null descriptor
    dq 0x0000000000000000

    ; 32-bit code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; 32-bit data segment
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


; ========================================
; Constants
; ========================================

CODE_SELECTOR equ 0x08
DATA_SELECTOR equ 0x10


; ========================================
; Variables
; ========================================

boot_drive:
    db 0

current_sector:
    db 2

sectors_left:
    db 16


; ========================================
; Diagnostic messages
; ========================================

msg_a:
    db 'A', 0

msg_b:
    db 'B', 0

msg_c:
    db 'C', 0

msg_error:
    db 'X', 0


; ========================================
; Boot sector padding/signature
; ========================================

times 510-($-$$) db 0

dw 0xAA55
