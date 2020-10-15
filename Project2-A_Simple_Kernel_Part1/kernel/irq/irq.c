#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <stdio.h>
#include <assert.h>
#include <sbi.h>
#include <screen.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];
uintptr_t riscv_dtb;

void reset_irq_timer()
{
    ;
}

void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    ;
}

void handle_int(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    ;
}

void init_exception()
{
    ;
}

void handle_other(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    ;
}
