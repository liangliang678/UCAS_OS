#include <asm/sbidef.h>
#include <asm/sbiasm.h>

.equ os_size_loc, 0x502001fc

//kernel address
.equ kernel, 0x50201000

//kernel main address
.equ kernel_main, 0x50201000

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
	SBI_CALL SBI_SD_READ

    j kernel_main

.data

msg: .string "It's a bootloader...\n\r"