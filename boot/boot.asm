; MatrixOS Bootloader
; Version 5.1 - Complete Floppy Kernel Loader

BITS 16
ORG 0x7C00

start:
    cli

    mov [boot_drive], dl

    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; ========================================
    ; Load MatrixOS kernel
    ;
    ; KERNEL_SECTORS is passed in by the Makefile
    ; (nasm -DKERNEL_SECTORS=N) from the real size
    ; of kernel.bin, so it never needs hand-editing.
    ;
    ; The kernel starts at LBA 1 (right after this
    ; boot sector) and is read one sector at a time,
    ; converting LBA -> CHS for a 1.44MB floppy
    ; (18 sectors/track, 2 heads), so it can span
    ; any number of tracks and cylinders.
    ;
    ; Kernel destination = 0000:1000
    ; ========================================

%ifndef KERNEL_SECTORS
    %error "KERNEL_SECTORS not defined - build with the Makefile"
%endif

    xor ax, ax
    mov es, ax

    mov bx, 0x1000              ; load address
    mov si, 1                   ; current LBA
    mov di, KERNEL_SECTORS      ; sectors left to read

.read_sector:
    mov ax, si                  ; LBA -> CHS
    xor dx, dx
    mov cx, 18
    div cx                      ; ax = LBA / 18 (track), dx = LBA % 18
    inc dl
    mov cl, dl                  ; sector (1-based)
    mov dh, al
    and dh, 1                   ; head = track & 1
    shr ax, 1
    mov ch, al                  ; cylinder = track >> 1
    mov dl, [boot_drive]

    mov bp, 3                   ; floppy reads can fail; try 3 times

.try:
    mov ax, 0x0201              ; read 1 sector
    int 0x13
    jnc .sector_ok

    push cx
    push dx
    xor ah, ah                  ; reset the disk and retry
    int 0x13
    pop dx
    pop cx

    dec bp
    jnz .try
    jmp disk_error

.sector_ok:
    add bx, 512
    inc si
    dec di
    jnz .read_sector

    ; ========================================
    ; VGA Mode 13h
    ; ========================================

    mov ax, 0x0013
    int 0x10

    ; ========================================
    ; Load GDT
    ; ========================================

    cli
    lgdt [gdt_descriptor]

    ; ========================================
    ; Enable protected mode
    ; ========================================

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; ========================================
    ; Far jump to protected mode
    ; ========================================

    db 0x66
    db 0xEA
    dd protected_mode
    dw CODE_SELECTOR


; ========================================
; Disk Error
; ========================================

disk_error:
    mov si, msg_error

.print:
    lodsb
    test al, al
    jz .hang

    mov ah, 0x0E
    mov bh, 0
    int 0x10

    jmp .print

.hang:
    cli
    hlt
    jmp .hang


; ========================================
; Protected Mode
; ========================================

BITS 32

protected_mode:

    mov ax, DATA_SELECTOR

    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x90000

    cld

    mov eax, 0x1000
    jmp eax


; ========================================
; GDT
; ========================================

BITS 16

gdt_start:

    dq 0

    ; Code
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

    ; Data
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


; ========================================
; Messages
; ========================================

msg_error:
    db 'X', 0


; ========================================
; Boot Signature
; ========================================

times 510-($-$$) db 0
dw 0xAA55
