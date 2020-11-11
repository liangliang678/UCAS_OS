HOST_CC = gcc
CROSS_PREFIX = riscv64-unknown-linux-gnu-
CC = ${CROSS_PREFIX}gcc
OBJDUMP = ${CROSS_PREFIX}objdump
CFLAGS = -O0  -fno-builtin -nostdlib -T riscv.lds -Wall -mcmodel=medany -Iinclude -Idrivers -Itest -Itest/test_project3 -Itiny_libc/include -nostdinc -g
DISK = /dev/sdb

BOOTLOADER_ENTRYPOINT = 0x50200000
KERNEL_ENTRYPOINT = 0x50201000

ARCH = riscv
ARCH_DIR = ./arch/$(ARCH)
CFLAGS += -Iarch/$(ARCH)/include

SRC_BOOT 	= $(ARCH_DIR)/boot/bootblock.S
SRC_ARCH	= $(ARCH_DIR)/kernel/trap.S $(ARCH_DIR)/kernel/entry.S $(ARCH_DIR)/kernel/smp.S $(ARCH_DIR)/kernel/head.S $(ARCH_DIR)/sbi/common.c
SRC_DRIVER	= ./drivers/screen.c
SRC_INIT 	= ./init/main.c
SRC_INT		= ./kernel/irq/irq.c
SRC_LOCK	= ./kernel/locking/lock.c ./kernel/locking/futex.c ./kernel/locking/binsem.c
SRC_SCHED	= ./kernel/sched/sched.c ./kernel/sched/time.c ./kernel/sched/smp.c
SRC_MM		= ./kernel/mm/mm.c
SRC_SYSCALL	= ./kernel/syscall/syscall.c ./kernel/syscall/mailbox.c
SRC_LIBS	= ./libs/string.c ./libs/printk.c

SRC_LIBC	= ./tiny_libc/getchar.c ./tiny_libc/printf.c ./tiny_libc/string.c ./tiny_libc/mthread.c ./tiny_libc/syscall.c ./tiny_libc/syscall.S ./tiny_libc/rand.c ./tiny_libc/time.c ./tiny_libc/mailbox.c

SRC_SHELL	= ./test/test_shell.c
SRC_TEST    = ./test/test_project3/test_affinity.c ./test/test_project3/test_barrier.c ./test/test_project3/test_condition.c ./test/test_project3/test_kill.c ./test/test_project3/test_multicore.c ./test/test_project3/test_sanguo.c ./test/test_project3/test_semaphore.c ./test/test_project3/test_wait.c
SRC_MAIN	= ${SRC_ARCH} ${SRC_INIT} ${SRC_INT} ${SRC_DRIVER} ${SRC_LOCK} ${SRC_SCHED} ${SRC_MM} ${SRC_SYSCALL} ${SRC_LIBS} ${SRC_LIBC} ${SRC_SHELL} ${SRC_TEST}

SRC_IMAGE	= ./tools/createimage.c

.PHONY:all main bootblock clean

all: createimage image asm

bootblock: $(SRC_BOOT) riscv.lds
	${CC} ${CFLAGS} -o bootblock $(SRC_BOOT) -e main -Ttext=${BOOTLOADER_ENTRYPOINT}

main: $(SRC_MAIN) riscv.lds
	${CC} ${CFLAGS} -o main $(SRC_MAIN) -Ttext=${KERNEL_ENTRYPOINT}

createimage: $(SRC_IMAGE)
	${HOST_CC} ${SRC_IMAGE} -o createimage -ggdb -Wall

image: bootblock main createimage
	./createimage --extended bootblock main

clean:
	rm -rf bootblock image createimage main
	find . -name "*.o" -exec rm {} \;

floppy:
	sudo fdisk -l ${DISK}
	sudo dd if=image of=${DISK}2 conv=notrunc

asm:
	${OBJDUMP} -d main > kernel.txt
