#include <asm/sbidef.h>
#include <asm/sbiasm.h>
#include <csr.h>

// start address
.equ kernel, 0x50201000

// start main address
.equ start, 0x50201000

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
	li t0, kernel		// dest mem addr
	li t1, os_size_loc	
	lh t1, 0(t1)		// remain num of blocks
	li t2, 1			// first block
	li t3, 64
	li t4, 32768
read:
	ble t1, t3, read_done
	mv a0, t0
	li a1, 64
	mv a2, t2
	SBI_CALL SBI_SD_READ
	add t0, t0, t4
	addi t1, t1, -64
	addi t2, t2, 64
	j read
read_done:
	mv a0, t0
	mv a1, t1
	mv a2, t2
	SBI_CALL SBI_SD_READ

    j start

secondary:
	// Mask all interrupts
	li t0, SR_SIE
  	csrc CSR_SSTATUS, t0
	csrw CSR_SIE, zero
	// let stvec pointer to start
	li t0, start
	csrw CSR_STVEC, t0
	// enable software interrupt for ipi
	li t0, SIE_SSIE
	csrs CSR_SIE, t0
	li t0, SR_SIE
  	csrs CSR_SSTATUS, t0
	
wait_for_wakeup:
	//wfi
	j wait_for_wakeup

.data
msg: .string "It's a bootloader...\n\r"