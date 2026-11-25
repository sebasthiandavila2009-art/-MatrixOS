# MatrixOS Build System
# Version 0.7 - Graphics

ASM = nasm
CC = clang
# Use ld.lld from PATH (Linux, or macOS with lld on PATH); fall back to Homebrew.
# (LD is a built-in make variable, so test its origin instead of using ?=.)
ifeq ($(origin LD),default)
LD = $(shell command -v ld.lld || echo "$$(brew --prefix lld)/bin/ld.lld")
endif

BUILD = build
IMAGE = $(BUILD)/matrixos.img

.PHONY: all run clean

all: $(IMAGE)

$(BUILD):
	mkdir -p $(BUILD)

# The bootloader is told how many sectors to load from the real size of
# kernel.bin, so it can never fall out of sync with the kernel again.
$(BUILD)/boot.bin: boot/boot.asm $(BUILD)/kernel.bin | $(BUILD)
	$(ASM) -f bin -DKERNEL_SECTORS=$$(( ( $$(wc -c < $(BUILD)/kernel.bin) + 511 ) / 512 )) \
		boot/boot.asm -o $(BUILD)/boot.bin

$(BUILD)/entry.o: kernel/entry.asm | $(BUILD)
	nasm -f elf32 kernel/entry.asm -o $(BUILD)/entry.o

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c kernel/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/keyboard.o: drivers/keyboard.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c drivers/keyboard.c -o $(BUILD)/keyboard.o

$(BUILD)/mouse.o: drivers/mouse.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c drivers/mouse.c -o $(BUILD)/mouse.o

$(BUILD)/graphics.o: drivers/graphics.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c drivers/graphics.c -o $(BUILD)/graphics.o

$(BUILD)/matrixfs.o: kernel/matrixfs.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c kernel/matrixfs.c -o $(BUILD)/matrixfs.o

$(BUILD)/kernel.bin: $(BUILD)/entry.o $(BUILD)/kernel.o $(BUILD)/matrixfs.o $(BUILD)/keyboard.o $(BUILD)/mouse.o $(BUILD)/graphics.o
	$(LD) -flavor gnu \
		-N \
		-e _start \
		-Ttext 0x1000 \
		--image-base 0x0 \
		--oformat binary \
		-o $(BUILD)/kernel.bin \
		$(BUILD)/entry.o \
		$(BUILD)/kernel.o \
		$(BUILD)/matrixfs.o \
		$(BUILD)/keyboard.o \
		$(BUILD)/mouse.o \
		$(BUILD)/graphics.o

$(IMAGE): $(BUILD)/boot.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$(IMAGE) bs=512 count=2880
	dd if=$(BUILD)/boot.bin of=$(IMAGE) conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$(IMAGE) bs=512 seek=1 conv=notrunc

run: $(IMAGE)
	qemu-system-x86_64 -drive file=$(IMAGE),format=raw,if=floppy

clean:
	rm -rf $(BUILD)
