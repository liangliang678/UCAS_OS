#include <os/irq.h>
#include <os/time.h>
#include <os/sched.h>
#include <os/string.h>
#include <stdio.h>
#include <assert.h>
#include <sbi.h>
#include <screen.h>
#include <csr.h>

handler_t irq_table[IRQC_COUNT];
handler_t exc_table[EXCC_COUNT];
uintptr_t riscv_dtb;

/* initialize irq_table and exc_table */
void init_exception()
{
    irq_table[IRQC_U_SOFT] = (handler_t)handle_other;
    irq_table[IRQC_S_SOFT] = (handler_t)handle_other;
    irq_table[IRQC_M_SOFT] = (handler_t)handle_other;
    irq_table[IRQC_U_TIMER] = (handler_t)handle_other;
    irq_table[IRQC_S_TIMER] = (handler_t)handle_int;
    irq_table[IRQC_M_TIMER] = (handler_t)handle_other;
    irq_table[IRQC_U_EXT] = (handler_t)handle_other;
    irq_table[IRQC_S_EXT] = (handler_t)handle_other;
    irq_table[IRQC_M_EXT] = (handler_t)handle_other;

    exc_table[EXCC_INST_MISALIGNED] = (handler_t)handle_other;
    exc_table[EXCC_INST_ACCESS] = (handler_t)handle_other;
    exc_table[EXCC_INST_ILLEGAL] = (handler_t)handle_other;
    exc_table[EXCC_BREAKPOINT] = (handler_t)handle_other;
    exc_table[EXCC_LOAD_ACCESS] = (handler_t)handle_other;
    exc_table[EXCC_STORE_ACCESS] = (handler_t)handle_other;
    exc_table[EXCC_SYSCALL] = (handler_t)handle_syscall;
    exc_table[EXCC_INST_PAGE_FAULT] = (handler_t)handle_other;
    exc_table[EXCC_LOAD_PAGE_FAULT] = (handler_t)handle_other;
    exc_table[EXCC_STORE_PAGE_FAULT] = (handler_t)handle_other;
    
    setup_exception();
}

// interrupt handler.
void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    // call corresponding handler by the value of `cause`
    if(cause & SCAUSE_IRQ_FLAG){
        cause = cause & ~SCAUSE_IRQ_FLAG;
        (*irq_table[cause])(regs, stval, cause);
    }
    else{
        (*exc_table[cause])(regs, stval, cause);
    }
}

void handle_int(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    reset_irq_timer();
}

void handle_other(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    char* reg_name[] = {
        "zero "," ra  "," sp  "," gp  "," tp  ",
        " t0  "," t1  "," t2  ","s0/fp"," s1  ",
        " a0  "," a1  "," a2  "," a3  "," a4  ",
        " a5  "," a6  "," a7  "," s2  "," s3  ",
        " s4  "," s5  "," s6  "," s7  "," s8  ",
        " s9  "," s10 "," s11 "," t3  "," t4  ",
        " t5  "," t6  "
    };
    for (int i = 0; i < 32; i += 3) {
        for (int j = 0; j < 3 && i + j < 32; ++j) {
            printk("%s : %016lx ",reg_name[i+j], regs->regs[i+j]);
        }
        printk("\n\r");
    }
    printk("sstatus: 0x%lx stval: 0x%lx scause: %lu\n\r",
           regs->sstatus, regs->stval, regs->scause);
    printk("sepc: 0x%lx\n\r", regs->sepc);
    assert(0);
}

//clock interrupt handler.
void reset_irq_timer()
{
    screen_reflush();
    timer_check();

    // use sbi_set_timer and reschedule
    sbi_set_timer(get_ticks() + timer_interval); 
    scheduler();
}