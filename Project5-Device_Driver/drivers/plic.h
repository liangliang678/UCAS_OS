#ifndef PLIC_H
#define PLIC_H

extern int plic_init(uintptr_t plic_regs_addr, u32 nr_irqs);

/*
 * Handling an interrupt is a two-step process: first you claim the interrupt
 * by reading the claim register, then you complete the interrupt by writing
 * that source ID back to the same claim register.  This automatically enables
 * and disables the interrupt, so there's nothing else to do.
 */
struct pt_regs;
extern void plic_handle_irq(regs_context_t *regs);

/* end of irq handling */
extern void plic_irq_eoi(int hwirq);

#endif //PLIC_H
