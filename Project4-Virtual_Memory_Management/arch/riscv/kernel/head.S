/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

.equ boot_stack, 0x50400000

.section ".entry_function","ax"
ENTRY(_start)
  /* setup C environment for boot.c */
  li sp, boot_stack
  
  /* this should never return */
  csrr a0, CSR_MHARTID
  call boot_kernel

END(_start)
