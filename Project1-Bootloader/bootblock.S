#include <asm/sbidef.h>
#include <asm/sbiasm.h>

// #define BOOT_LOADER_SIG_OFFSET 0x1fe
// #define OS_SIZE_LOC (BOOT_LOADER_SIG_OFFSET - 2)
// #define OS_SIZE_LOC 0x1fc

.equ os_size_loc, 0x502001fc

//kernel address (move kernel to here ~)
.equ kernel, 0x50200000

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// print a \n to avoid system output error...
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR

	// 1) task1 call BIOS print string "It's bootblock!"
	la a0, msg
	SBI_CALL SBI_CONSOLE_PUTSTR

	// 2) task2 call BIOS read kernel in SD card and jump to kernel start
	li a0, kernel
	li t0, os_size_loc
	lh a1, 0(t0)
	li a2, 1
	li a7, SBI_SD_READ
	ecall

.data

msg: .string "It's a bootloader...\n\r"
