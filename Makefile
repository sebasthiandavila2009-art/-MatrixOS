# MatrixOS Build System
# Version 0.1

ASM = nasm
BUILD = build
IMAGE = $(BUILD)/matrixos.img

.PHONY: all run clean

all: $(IMAGE)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.bin: boot/boot.asm | $(BUILD)
	$(ASM) -f bin boot/boot.asm -o $(BUILD)/boot.bin

$(IMAGE): $(BUILD)/boot.bin
	dd if=/dev/zero of=$(IMAGE) bs=512 count=2880
	dd if=$(BUILD)/boot.bin of=$(IMAGE) conv=notrunc

run: $(IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(IMAGE)

clean:
	rm -rf $(BUILD)
