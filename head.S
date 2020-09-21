/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
*/
#define CSR_SIE			0x104
#define CSR_SIP			0x144
#define KERNEL_STACK		0x50500000

//kernel main address (jmp here to start kernel main!)
.equ kernel_main, 0x50200000

.section ".entry_function","ax"
.global _start
_start:  
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  
  /* Clear BSS for flat non-ELF images */
	la a0, __bss_start
	la a1, __BSS_END__
	ble a1, a0, clear_bss_done
clear_bss:
	sb zero, 0(a0)
	addi a0, a0, 1
	blt a0, a1, clear_bss
clear_bss_done:

  /* setup C environment (set sp register)*/
  li sp, KERNEL_STACK

  /* Jump to the code in kernel.c*/
  j main

  nop
  nop
  nop
  nop
  
  /* execute following inst after ecall in bootblock.S:34 */
  nop
  fence.i
  j kernel_main
