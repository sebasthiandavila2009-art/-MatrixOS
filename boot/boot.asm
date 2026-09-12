; MatrixOS Bootloader
; Version 2.0 - Reliable 30 Sector CHS Loader

BITS 16
ORG 0x7C00

start:

    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x7C00

    mov [boot_drive], dl

    ; ----------------------------------------
    ; A = Bootloader started
    ; ----------------------------------------

    mov al, 'A'
    call print_char

    ; ----------------------------------------
    ; Reset disk
    ; ----------------------------------------

    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; ----------------------------------------
    ; B = Disk reset works
    ; ----------------------------------------

    mov al, 'B'
    call print_char

    ; ----------------------------------------
    ; Get BIOS disk geometry
    ; ----------------------------------------

    mov ah, 0x08
    mov dl, [boot_drive]
    int 0x13
    jc disk_error

    ; Sectors per track
    mov al, cl
    and al, 0x3F
    mov [sectors_per_track], al

    ; Maximum head
    mov [max_head], dh

    ; ----------------------------------------
    ; Kernel destination
    ; 0000:1000
    ; ----------------------------------------

    mov ax, 0x1000
    mov es, ax

    xor bx, bx

    ; ----------------------------------------
    ; Start CHS position
    ;
    ; Cylinder 0
    ; Head 0
    ; Sector 2
    ;
    ; Sector 1 contains bootloader
    ; ----------------------------------------

    mov byte [current_sector], 2
    mov byte [current_head], 0
    mov word [current_cylinder], 0

    ; 30 sectors total
    mov byte [sectors_left], 30


load_sector:

    ; ----------------------------------------
    ; Read ONE sector
    ; ----------------------------------------

    mov ah, 0x02
    mov al, 1

    mov ch, byte [current_cylinder]

    mov cl, byte [current_sector]

    mov dh, byte [current_head]

    mov dl, [boot_drive]

    int 0x13
    jc read_retry

    ; ----------------------------------------
    ; Sector loaded
    ; ----------------------------------------

    dec byte [sectors_left]

    jz kernel_loaded

    ; ----------------------------------------
    ; Move to next sector
    ; ----------------------------------------

    inc byte [current_sector]

    mov al, [sectors_per_track]
    cmp byte [current_sector], al
    jbe load_sector

    ; ----------------------------------------
    ; New track
    ; ----------------------------------------

    mov byte [current_sector], 1

    inc byte [current_head]

    mov al, [max_head]
    cmp byte [current_head], al
    jbe load_sector

    ; ----------------------------------------
    ; New cylinder
    ; ----------------------------------------

    mov byte [current_head], 0
    inc word [current_cylinder]

    jmp load_sector


read_retry:

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    ; Retry the same sector
    mov ah, 0x02
    mov al, 1

    mov ch, byte [current_cylinder]
    mov cl, byte [current_sector]
    mov dh, byte [current_head]
    mov dl, [boot_drive]

    int 0x13
    jc disk_error

    dec byte [sectors_left]

    jz kernel_loaded

    inc byte [current_sector]

    mov al, [sectors_per_track]
    cmp byte [current_sector], al
    jbe load_sector

    mov byte [current_sector], 1
    inc byte [current_head]

    mov al, [max_head]
    cmp byte [current_head], al
    jbe load_sector

    mov byte [current_head], 0
    inc word [current_cylinder]

    jmp load_sector


kernel_loaded:

    ; ----------------------------------------
    ; C = Entire 30-sector kernel loaded
    ; ----------------------------------------

    mov al, 'C'
    call print_char

    ; ----------------------------------------
    ; Enter protected mode
    ; ----------------------------------------

    cli

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode


disk_error:

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


; ========================================
; Protected Mode
; ========================================

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


; ========================================
; GDT
; ========================================

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


; ========================================
; Variables
; ========================================

boot_drive:
    db 0

sectors_per_track:
    db 18

max_head:
    db 1

current_sector:
    db 2

current_head:
    db 0

current_cylinder:
    dw 0

sectors_left:
    db 30


error_message:

    db "MatrixOS: Disk error", 0


; ========================================
; Boot Signature
; ========================================

times 510 - ($ - $$) db 0

dw 0xAA55
