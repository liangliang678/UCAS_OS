#include <sbi.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>

struct spin_lock kernel_lock;

void smp_init()
{
    /* TODO: */
    set_tp((uint64_t)&kernel_pcb[1]);
    enable_interrupt();
    setup_exception();
    // sbi_set_timer(get_ticks() + timer_interval);
    while (1){
        __asm__ __volatile__("wfi\n\r":::);
    }
}

void wakeup_other_hart()
{
    /* TODO: */
    unsigned long hart_mask = 0;
    sbi_send_ipi(&hart_mask);
}

void lock_kernel()
{
    /* TODO: */
    spin_lock_acquire(&kernel_lock);
}

void unlock_kernel()
{
    /* TODO: */
    spin_lock_release(&kernel_lock);
}

