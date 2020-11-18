/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

/* maybe you need another stack for booting? */
.equ boot_stack_top_base, 0x50202000

.section ".entry_function","ax"
ENTRY(_start)
  /* TODO: */
  /* setup C environment for boot.c */

  /* this should never return */
  call boot_kernel

loop:
  wfi
  j loop

END(_start)
