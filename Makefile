# MatrixOS Build System
# Version 0.5

ASM = nasm
CC = clang
LD = $(shell brew --prefix lld)/bin/ld.lld

BUILD = build
IMAGE = $(BUILD)/matrixos.img

.PHONY: all run clean

all: $(IMAGE)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.bin: boot/boot.asm | $(BUILD)
	$(ASM) -f bin boot/boot.asm -o $(BUILD)/boot.bin

$(BUILD)/entry.o: kernel/entry.asm | $(BUILD)
	$(ASM) -f elf32 kernel/entry.asm -o $(BUILD)/entry.o

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c kernel/kernel.c -o $(BUILD)/kernel.o

$(BUILD)/keyboard.o: drivers/keyboard.c | $(BUILD)
	$(CC) -target i386-unknown-none -ffreestanding \
		-fno-stack-protector -fno-pic -fno-pie \
		-m32 -c drivers/keyboard.c -o $(BUILD)/keyboard.o

$(BUILD)/kernel.bin: $(BUILD)/entry.o $(BUILD)/kernel.o $(BUILD)/keyboard.o
	$(LD) -flavor gnu \
		-e _start \
		-Ttext 0x1000 \
		--image-base 0x0 \
		--oformat binary \
		-o $(BUILD)/kernel.bin \
		$(BUILD)/entry.o \
		$(BUILD)/kernel.o \
		$(BUILD)/keyboard.o

$(IMAGE): $(BUILD)/boot.bin $(BUILD)/kernel.bin
	dd if=/dev/zero of=$(IMAGE) bs=512 count=2880
	dd if=$(BUILD)/boot.bin of=$(IMAGE) conv=notrunc
	dd if=$(BUILD)/kernel.bin of=$(IMAGE) bs=512 seek=1 conv=notrunc

run: $(IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf $(BUILD)
