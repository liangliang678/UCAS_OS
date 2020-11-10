#include <sbi.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>

volatile int init_flag = 0;
struct spin_lock kernel_lock;

void smp_init()
{
    /* TODO: */
    while(!init_flag){
        setup_exception();
        sbi_set_timer(get_ticks() + timer_interval);
        enable_interrupt();
        printk("> [INIT] smp initialization succeeded.\n\r");
    }
}

void wakeup_other_hart()
{
    /* TODO: */
    unsigned long hart_mask = 1;
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

