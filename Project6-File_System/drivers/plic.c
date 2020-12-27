// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017 SiFive
 * Copyright (C) 2018 Christoph Hellwig
 */

/*
 * This driver implements a version of the RISC-V PLIC with the actual layout
 * specified in chapter 8 of the SiFive U5 Coreplex Series Manual:
 *
 *     https://static.dev.sifive.com/U54-MC-RVCoreIP.pdf
 *
 * The largest number supported by devices marked as 'sifive,plic-1.0.0', is
 * 1024, of which device 0 is defined as non-existent by the RISC-V Privileged
 * Spec.
 */

#include <type.h>
#include <io.h>
#include <csr.h>
#include <os/irq.h>
#include <stdio.h>
#include <plic.h>

#define MAX_DEVICES			1024
#define MAX_CONTEXTS			15872

/*
 * Each interrupt source has a priority register associated with it.
 * We always hardwire it to one in Linux.
 */
#define PRIORITY_BASE			0
#define     PRIORITY_PER_ID		4

/*
 * Each hart context has a vector of interrupt enable bits associated with it.
 * There's one bit for each interrupt source.
 */
#define ENABLE_BASE			0x2000
#define     ENABLE_PER_HART		0x80

/*
 * Each hart context has a set of control registers associated with it.  Right
 * now there's only two: a source priority threshold over which the hart will
 * take an interrupt, and a register to claim interrupts.
 */
#define CONTEXT_BASE			0x200000
#define     CONTEXT_PER_HART		0x1000
#define     CONTEXT_THRESHOLD		0x00
#define     CONTEXT_CLAIM		0x04

static void *plic_regs;

struct plic_handler {
	bool			present;
	void		*hart_base;
	void		*enable_base;
} plic_handlers;

static inline void plic_toggle(struct plic_handler *handler,
				int hwirq, int enable)
{
	u32 *reg = handler->enable_base + (hwirq / 32) * sizeof(u32);
	u32 hwirq_mask = 1 << (hwirq % 32);

	if (enable)
		writel(readl(reg) | hwirq_mask, reg);
	else
		writel(readl(reg) & ~hwirq_mask, reg);
}

static void plic_irq_unmask(int hwirq)
{
	int enable = 1;
	writel(enable, plic_regs + PRIORITY_BASE + hwirq * PRIORITY_PER_ID);
    struct plic_handler *handler = &plic_handlers;

    if (handler->present) plic_toggle(handler, hwirq, enable);
}

/*
static void plic_irq_mask(int hwirq)
{
	int enable = 0;
	writel(enable, plic_regs + PRIORITY_BASE + hwirq * PRIORITY_PER_ID);
    struct plic_handler *handler = &plic_handlers;

    if (handler->present) plic_toggle(handler, hwirq, enable);
}
*/

void plic_irq_eoi(int hwirq)
{
	struct plic_handler *handler = &plic_handlers;

	writel(hwirq, handler->hart_base + CONTEXT_CLAIM);
}

#define __ASM_STR(x)	#x
#define csr_set(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrs " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			\
			      : "memory");			\
})

#define csr_clear(csr, val)					\
({								\
	unsigned long __v = (unsigned long)(val);		\
	__asm__ __volatile__ ("csrc " __ASM_STR(csr) ", %0"	\
			      : : "rK" (__v)			\
			      : "memory");			\
})

/*
 * Handling an interrupt is a two-step process: first you claim the interrupt
 * by reading the claim register, then you complete the interrupt by writing
 * that source ID back to the same claim register.  This automatically enables
 * and disables the interrupt, so there's nothing else to do.
 */
void plic_handle_irq(regs_context_t *regs)
{
	struct plic_handler *handler = &plic_handlers;
	void *claim = handler->hart_base + CONTEXT_CLAIM;
	int hwirq;

	csr_clear(sie, SIE_SEIE);
	while ((hwirq = readl(claim))) {
			handle_irq(regs, hwirq);
	}
	csr_set(sie, SIE_SEIE);
}

int plic_init(uintptr_t plic_regs_addr, u32 nr_irqs)
{
    plic_regs = (void *)plic_regs_addr;

    struct plic_handler *handler;
    int hwirq;
    u32 threshold = 0;

    handler = &plic_handlers;
    if (handler->present) {
        printk("handler already present.\n");
        threshold = 0xffffffff;
        goto done;
    }

    handler->present     = true;
    handler->hart_base   = plic_regs + CONTEXT_BASE;
    handler->enable_base = plic_regs + ENABLE_BASE;

done:
    /* priority must be > threshold to trigger an interrupt */
    writel(threshold, handler->hart_base + CONTEXT_THRESHOLD);
    for (hwirq = 1; hwirq <= nr_irqs; hwirq++) plic_toggle(handler, hwirq, 1);

    // for qemu to interrupt
    plic_irq_unmask(33);

    return 0;
}

