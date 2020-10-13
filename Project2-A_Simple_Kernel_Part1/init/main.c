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
#include <screen.h>
#include <sbi.h>
#include <stdio.h>
#include <os/time.h>
#include <os/syscall.h>
#include <os/futex.h>
#include <test.h>

#include <csr.h>

extern void ret_from_exception();
extern void __global_pointer$();

static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, 
    ptr_t entry_point, pcb_t *pcb)
{
    regs_context_t *pt_regs = (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pcb->kernel_sp -= sizeof(regs_context_t);
    /* initialization registers (ra, sp, gp, sepc, sstatus)
     * To run the task in user mode,
     * you should set corresponding bits of sstatus(SPP, SPIE, etc.).
     */
    /* prepare a stack, and push some values to simulate a pcb context */
    pt_regs->regs[1] = entry_point;             //ra
    pt_regs->regs[2] = user_stack;              //sp
    pt_regs->regs[3] = (reg_t)__global_pointer$;//gp
}

static void init_pcb()
{
    /* initialize all pcb and add them into ready_queue */
    int num_tasks; 
    
    for(num_tasks = 0; num_tasks < num_sched1_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = pcb[num_tasks].kernel_sp;
        pcb[num_tasks].preempt_count = 0;
        list_add(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = sched1_tasks[num_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        sched1_tasks[num_tasks]->entry_point, &pcb[num_tasks]); 
    }
    
    for(num_tasks = num_sched1_tasks; num_tasks < num_sched1_tasks + num_lock_tasks; num_tasks++){
        pcb[num_tasks].kernel_sp = allocPage(1);
        pcb[num_tasks].user_sp = pcb[num_tasks].kernel_sp;
        pcb[num_tasks].preempt_count = 0;
        list_add(&pcb[num_tasks].list, &ready_queue);
        pcb[num_tasks].pid = num_tasks + 1;
        pcb[num_tasks].type = lock_tasks[num_tasks- num_sched1_tasks]->type;
        pcb[num_tasks].status = TASK_READY;
        
        init_pcb_stack( pcb[num_tasks].kernel_sp, pcb[num_tasks].user_sp, 
                        lock_tasks[num_tasks - num_sched1_tasks]->entry_point, &pcb[num_tasks]); 
    }

    /* initialize `current_running` */
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // initialize system call table.
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
    // init_exception();
    // printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table
    // init_syscall();
    // printk("> [INIT] System call initialized successfully.\n\r");

    // init screen
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // TODO:
    // Setup timer interrupt and enable all interrupt

    while (1) {
        // If you do non-preemptive scheduling, you need to use it to surrender control do_scheduler();
        // enable_interrupt();
        // __asm__ __volatile__("wfi\n\r":::);
        do_scheduler();
    };
    return 0;
}
