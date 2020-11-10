#include <asm/sbidef.h>
#include <asm/sbiasm.h>
#include <csr.h>

// kernel address
.equ kernel, 0x50201000

// kernel main address
.equ kernel_main, 0x50201000

// os size location
.equ os_size_loc, 0x502001fc

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// a0 is mhartid, a1 is the pointer of fdt
	move s0,a0
	move s1,a1

	// output a \n
	li a0, 10
	SBI_CALL SBI_CONSOLE_PUTCHAR

	// The core whose mhartid=0 is selected to boot the kernel.
	// Other cores should wait until the kernel's initialization has been finished.
 	move a0, s0
	bnez a0, secondary

	// call BIOS read kernel in SD card and jump to kernel start
	li a0, kernel
	li t0, os_size_loc
	lh a1, 0(t0)
	li a2, 1
	SBI_CALL SBI_SD_READ

    j kernel_main

secondary:
	/* TODO: 
	 * 1. Mask all interrupts
	 * 2. let stvec pointer to kernel_main
	 * 3. enable software interrupt for ipi
	 */
	csrw CSR_SIE, zero
	ld t0, kernel_main
	csrw CSR_STVEC, t0
	li t0, SIE_SSIE
	csrs CSR_SIE, t0
	
wait_for_wakeup:
	wfi
	j wait_for_wakeup

// while(1) --> stop here
stop:
	j stop
	nop

.data
msg: .string "It's a bootloader...\n\r"