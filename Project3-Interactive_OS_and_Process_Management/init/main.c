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
#include <os/binsem.h>
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
        pt_regs->sstatus = (reg_t)(SR_SPP | SR_SPIE);  //SPP = 1, SPIE = 1
        pt_regs->sepc = (reg_t)entry_point;
        pt_regs->sscratch = (reg_t)0;                  //sscratch = 0
    }
}

static void init_pcb()
{
    /* initialize all pcb and add test_shell into ready_queue */
    for(int i = 0; i < NUM_MAX_TASK; i++){
        pcb[i].pid = -1;
    }
    
    pcb[0].kernel_sp = allocPage(2);
    pcb[0].user_sp = allocPage(2);
    pcb[0].preempt_count = 0;
    pcb[0].kernel_stack_base = pcb[0].kernel_sp;
    pcb[0].user_stack_base = pcb[0].user_sp;
    list_add_tail(&pcb[0].list, &ready_queue);
    init_list_head(&pcb[0].wait_list);
    pcb[0].mutex_num = 0;
    pcb[0].pid = 1;
    pcb[0].type = USER_PROCESS;
    pcb[0].status = TASK_READY;
    pcb[0].mode = AUTO_CLEANUP_ON_EXIT;
    pcb[0].priority = 0;
    pcb[0].ready_tick = get_ticks();
        
    init_pcb_stack(pcb[0].kernel_sp, pcb[0].user_sp, &test_shell, &pcb[0]);

    init_list_head(&pid0_pcb.wait_list);

    /* initialize `current_running` */
    current_running = &pid0_pcb;
}

static void init_syscall(void)
{
    // initialize system call table.
    syscall[SYSCALL_SPAWN] = (long(*)())do_spawn;
    syscall[SYSCALL_EXIT] = (long(*)())do_exit;
    syscall[SYSCALL_SLEEP] = (long(*)())do_sleep;
    syscall[SYSCALL_KILL] = (long(*)())do_kill;
    syscall[SYSCALL_WAITPID] = (long(*)())do_waitpid;
    syscall[SYSCALL_PS] = (long(*)())do_process_show;
    syscall[SYSCALL_GETPID] = (long(*)())do_getpid;
    syscall[SYSCALL_YIELD] = (long(*)())0;
    syscall[SYSCALL_FUTEX_WAIT] = (long(*)())futex_wait;
    syscall[SYSCALL_FUTEX_WAKEUP] = (long(*)())futex_wakeup;
    syscall[SYSCALL_BINSEM_GET] = (long(*)())binsem_get;
    syscall[SYSCALL_BINSEM_OP] = (long(*)())binsem_op;
    syscall[SYSCALL_WRITE] = (long(*)())screen_write;
    syscall[SYSCALL_CURSOR] = (long(*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long(*)())screen_reflush;
    syscall[SYSCALL_SCREEN_CLEAR] = (long(*)())screen_clear;
    syscall[SYSCALL_SCREEN_SCROLL] = (long(*)())screen_scroll;
    syscall[SYSCALL_GET_TIMEBASE] = (long(*)())get_time_base;
    syscall[SYSCALL_GET_TICK] = (long(*)())get_ticks;   
    syscall[SYSCALL_GET_CHAR] = (long(*)())sbi_console_getchar;  
}

// jump from bootloader
// The beginning of everything
int main()
{
    // init Process Control Block
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n\r");

    // read CPU frequency and calc timer interval
    time_base = sbi_read_fdt(TIMEBASE);
    timer_interval = (uint64_t)(time_base / 50);
	
    // init futex mechanism and binsem mechanism
    init_system_futex();
    init_system_binsem();

    // init interrupt
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n\r");

    // init system call table
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n\r");

    // init screen
    init_screen();
    vt100_move_cursor(1, 1);
    printk("> [INIT] SCREEN initialization succeeded.\n\r");

    // Setup timer interrupt and enable all interrupt
    sbi_set_timer(get_ticks() + timer_interval);
    enable_interrupt();

    while (1) {
        list_node_t* clean_p = pid0_pcb.wait_list.next;
        while(clean_p != &pid0_pcb.wait_list){
            pcb_t *clean_pcb = list_entry(clean_p, pcb_t, list);
            // TODO: release kernel stack

            // release pcb
            clean_pcb->status = TASK_EXITED;
            clean_pcb->pid = -1;

            clean_p = clean_p->next;
            list_del(clean_p->prev);
        }
        __asm__ __volatile__("wfi\n\r":::);
    }
    
    return 0;
}