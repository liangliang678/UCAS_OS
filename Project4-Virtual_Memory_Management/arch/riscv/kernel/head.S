/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

.equ boot_stack_top_base, 0x50400000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* Disable FPU to detect illegal usage of floating point in kernel space */
  li t0, SR_FS
  csrc sstatus, t0

  /* Load the global pointer */
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  #csrr t0, CSR_MHARTID
  #bnez t0, secondary
  
  /* Clear BSS for flat non-ELF images */
  la a3, __bss_start
  la a4, __BSS_END__
  ble a4, a3, clear_bss_done
clear_bss:
  sd zero, (a3)
  add a3, a3, RISCV_SZPTR
  blt a3, a4, clear_bss
clear_bss_done:

  /* setup C environment for boot.c */
  //ld tp, core0_tp
  li sp, boot_stack_top_base
  csrw CSR_SSCRATCH, zero

  //la a0, riscv_dtb
  //sd a1, (a0)
  csrr a0, CSR_MHARTID
  /* this should never return */
  call boot_kernel

loop:
  wfi
  j loop

END(_start)
