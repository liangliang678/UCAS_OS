#include <sbi.h>
#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/irq.h>

struct spin_lock kernel_lock;

void smp_init()
{
    enable_interrupt();
    setup_exception();
    sbi_set_timer(get_ticks() + timer_interval);
    while (1){
        __asm__ __volatile__("wfi\n\r":::);
    }
}

void wakeup_other_hart()
{
    sbi_send_ipi(NULL);
}

void lock_kernel()
{
    spin_lock_acquire(&kernel_lock);
}

void unlock_kernel()
{
    spin_lock_release(&kernel_lock);
}