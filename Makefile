CC = riscv64-unknown-linux-gnu-gcc
HOST_CC = gcc
# CFLAGS = -O2  -fno-pic -fno-builtin -nostdinc -N -o bootblock bootblock.s -nostdlib -e main -Wl,-m -T riscv.lds
CFLAGS = -O2  -fno-builtin -nostdlib -T riscv.lds -Iinclude -Wall -mcmodel=medany
DISK = /dev/sdb

BOOTLOADER_ENTRYPOINT = 0x50200000
KERNEL_ENTRYPOINT = 0x50200000

all: createimage image

bootblock: bootblock.S riscv.lds
	${CC} ${CFLAGS} -o bootblock bootblock.S -e main -Ttext=${BOOTLOADER_ENTRYPOINT}

kernel: kernel.c head.S riscv.lds
	${CC} ${CFLAGS} -o kernel kernel.c head.S -Ttext=${KERNEL_ENTRYPOINT}

createimage: createimage.c
	${HOST_CC} createimage.c -o createimage -ggdb -Wall

image: bootblock kernel createimage
	./createimage --extended bootblock kernel

clean:
	rm -rf bootblock image kernel *.o createimage

floppy:
	sudo fdisk -l ${DISK}
	sudo dd if=image of=${DISK}2 conv=notrunc
