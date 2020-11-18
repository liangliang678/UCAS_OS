/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>
#include <asm/sbidef.h>
#include <asm/sbiasm.h>

.section ".entry_function","ax"
ENTRY(_start)
  /* TODO: this is ther kernel's entry point.
   * Same as head.S in Project 3?
   */

END(_start)
