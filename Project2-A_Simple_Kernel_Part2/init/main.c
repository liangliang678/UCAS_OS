/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <common.h>
#include <os/irq.h>
#include <os/mm.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/syscall.h>
#include <os/futex.h>
#include <screen.h>
#include <sbi.h>
#include <stdio.h>
#include <test.h>
#include <csr.h>

extern void ret_from_exception();
extern void __global_pointer$();

static void init_pcb_stack(ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point, pcb_t *pcb)
{
    regs_context_t *pt_regs = (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pcb->kernel_sp -= sizeof(regs_context_t);

    /* initialization registers: ra, sp, gp, sepc, sstatus, sepc, sscratch */
    pt_regs->regs[1] = (reg_t)entry_point;      //ra
    pt_regs->regs[2] = (reg_t)user_stack;       //sp
    pt_regs->regs[3] = (reg_t)__global_pointer$;//gp
    if(pcb->type == USER_PROCESS || pcb->type == USER_THREAD){
        pt_regs->sstatus = (reg_t)SR_SPIE;      //SPP = 0, SPIE = 1
        pt_regs->sepc = (reg_t)entry_point;
        pt_regs->sscratch = (reg_t)pcb;         //sscratch = tp
    }
    else{
        pt_regs->sstatus = SR_SPP | SR_SPIE;    //SPP = 1, SPIE = 1
        pt_regs->sepc = entry_point;
        pt_regs->sscratch = 0;                  //sscratch = 0
    }
}

static void init_pcb()
{
    /* initialize all pcb and add them into ready_queue */
    int num_tasks;
/*
    // KERNEL
    // sched1_tasks
    for(num_tasks = 0; num_tasks < num_sched1_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = pcb[num_tasks].kernel_sp;
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = sched1_tasks[num_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = sched1_tasks[num_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        sched1_tasks[num_tasks]->entry_point, &pcb[num_tasks]); 
    }
    //lock_tasks
    for(num_tasks = num_sched1_tasks; num_tasks < num_sched1_tasks + num_lock_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = pcb[num_tasks].kernel_sp;
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = lock_tasks[num_tasks - num_sched1_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = lock_tasks[num_tasks - num_sched1_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        lock_tasks[num_tasks - num_sched1_tasks]->entry_point, &pcb[num_tasks]); 
    }
*/
    // USER
    // timer_tasks
    for(num_tasks = 0; num_tasks < num_timer_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = allocPage(1);
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = timer_tasks[num_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = timer_tasks[num_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        timer_tasks[num_tasks]->entry_point, &pcb[num_tasks]); 
    }
    // sched2_tasks
    for(num_tasks = num_timer_tasks; num_tasks < num_timer_tasks + num_sched2_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = allocPage(1);
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = sched2_tasks[num_tasks - num_timer_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = sched2_tasks[num_tasks - num_timer_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        sched2_tasks[num_tasks - num_timer_tasks]->entry_point, &pcb[num_tasks]); 
    }
    // lock2_tasks
    for(num_tasks = num_timer_tasks + num_sched2_tasks; num_tasks < num_timer_tasks + num_sched2_tasks + num_lock2_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = allocPage(1);
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + num_timer_tasks + num_sched2_tasks + 1;
        pcb[num_tasks].type = lock2_tasks[num_tasks - num_timer_tasks - num_sched2_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = lock2_tasks[num_tasks - num_timer_tasks - num_sched2_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        lock2_tasks[num_tasks - num_timer_tasks - num_sched2_tasks]->entry_point, &pcb[num_tasks]); 
    }
/*
    // PRIORITY
    // priority_tasks
    for(num_tasks = 0; num_tasks < num_priority_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = allocPage(1);
        pcb[num_tasks].preempt_count = 0;
        list_add_tail(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = priority_tasks[num_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        pcb[num_tasks].priority = priority_tasks[num_tasks]->priority;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        priority_tasks[num_tasks]->entry_point, &pcb[num_tasks]); 
    }
*/
    /* initialize `current_running` */
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // initialize system call table.
    syscall[SYSCALL_SLEEP] = (long(*)())do_sleep;
    syscall[SYSCALL_FUTEX_WAIT] = (long(*)())futex_wait;
    syscall[SYSCALL_FUTEX_WAKEUP] = (long(*)())futex_wakeup;
    syscall[SYSCALL_WRITE] = (long(*)())screen_write;
    syscall[SYSCALL_READ] = (long(*)())handle_other;
    syscall[SYSCALL_CURSOR] = (long(*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long(*)())screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE] = (long(*)())get_time_base;
    syscall[SYSCALL_GET_TICK] = (long(*)())get_ticks;
}

// jump from bootloader
// The beginning of everything
int main()
{
    // init Process Control Block
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // read CPU frequency
    time_base = sbi_read_fdt(TIMEBASE);
	
    // init futex mechanism
    init_system_futex();

    // init interrupt
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // init screen
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // Setup timer interrupt and enable all interrupt
    sbi_set_timer(get_ticks() + TIMER_INTERVAL);
    enable_interrupt();

    while (1) {
        __asm__ __volatile__("wfi\n\r":::);
    }
    
    return 0;
}
