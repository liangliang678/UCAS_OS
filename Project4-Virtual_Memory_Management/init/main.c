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
#include <os/mailbox.h>
#include <os/smp.h>
#include <os/elf.h>
#include <screen.h>
#include <sbi.h>
#include <pgtable.h>
#include <stdio.h>
#include <csr.h>

#include <user_programs.h>

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
    pt_regs->sstatus = (reg_t)SR_SPIE;          //SPP = 0, SPIE = 1
    pt_regs->sepc = (reg_t)entry_point;         //sepc
    pt_regs->sscratch = (reg_t)pcb;             //sscratch = tp
}

static void init_pcb()
{
    kernel_pcb[0].kernel_sp = (ptr_t)kernel_stack_0;
    kernel_pcb[0].user_sp = (ptr_t)kernel_stack_0;
    kernel_pcb[0].preempt_count = 0;
    kernel_pcb[0].kernel_stack_base = (ptr_t)kernel_stack_0;
    kernel_pcb[0].user_stack_base = (ptr_t)kernel_stack_0;
    kernel_pcb[0].binsem_num = 0;
    kernel_pcb[0].pid = 0;
    kernel_pcb[0].type = KERNEL_PROCESS;
    kernel_pcb[0].status = TASK_RUNNING;
    kernel_pcb[0].priority = 0;
    kernel_pcb[0].mask = 1;
    kernel_pcb[0].cpu_id = 0;
    kernel_pcb[0].pgdir = (PTE*)PGDIR_PA;
    init_list_head(&kernel_pcb[0].wait_list);

    kernel_pcb[1].kernel_sp = (ptr_t)kernel_stack_1;
    kernel_pcb[1].user_sp = (ptr_t)kernel_stack_1;
    kernel_pcb[1].preempt_count = 0;
    kernel_pcb[1].kernel_stack_base = (ptr_t)kernel_stack_1;
    kernel_pcb[1].user_stack_base = (ptr_t)kernel_stack_1;
    kernel_pcb[1].binsem_num = 0;
    kernel_pcb[1].pid = 0;
    kernel_pcb[1].type = KERNEL_PROCESS;
    kernel_pcb[1].status = TASK_RUNNING;
    kernel_pcb[1].priority = 0;
    kernel_pcb[1].mask = 2;
    kernel_pcb[1].cpu_id = 1;
    kernel_pcb[1].pgdir = (PTE*)PGDIR_PA;

    /* initialize all pcb and add test_shell into ready_queue */
    for(int i = 0; i < NUM_MAX_TASK; i++){
        pcb[i].pid = -1;
    }
    
    pcb[0].kernel_sp = pa2kva(alloc_user_page(1, NULL) + PAGE_SIZE);
    pcb[0].user_sp = USER_STACK_ADDR;
    pcb[0].preempt_count = 0;
    pcb[0].kernel_stack_base = pcb[0].kernel_sp;
    pcb[0].user_stack_base = pcb[0].user_sp;
    list_add_tail(&pcb[0].list, &ready_queue);
    init_list_head(&pcb[0].wait_list);
    pcb[0].binsem_num = 0;
    pcb[0].pid = 1;
    pcb[0].type = USER_PROCESS;
    pcb[0].status = TASK_READY;
    pcb[0].mode = AUTO_CLEANUP_ON_EXIT;
    pcb[0].priority = 0;
    pcb[0].ready_tick = get_ticks();    
    pcb[0].mask = 3; 
    pcb[0].pgdir = init_page_table();

    char *binary;
    int length;
    get_elf_file("shell", (unsigned char**)&binary, &length);
    uintptr_t entry = load_elf((unsigned char*)binary, length, (uintptr_t)pcb[0].pgdir, get_kva_of);

    init_pcb_stack((ptr_t)pcb[0].kernel_sp, (ptr_t)pcb[0].user_sp, entry, &pcb[0]);

    /* initialize `current_running` */
    current_running[0] = &kernel_pcb[0];
    current_running[1] = &kernel_pcb[1];
}

static void init_syscall(void)
{
    // initialize system call table.
    syscall[SYSCALL_EXEC] = (long(*)())do_exec;
    syscall[SYSCALL_EXIT] = (long(*)())do_exit;
    syscall[SYSCALL_SLEEP] = (long(*)())do_sleep;
    syscall[SYSCALL_KILL] = (long(*)())do_kill;
    syscall[SYSCALL_WAITPID] = (long(*)())do_waitpid;
    syscall[SYSCALL_PS] = (long(*)())do_process_show;
    syscall[SYSCALL_GETPID] = (long(*)())do_getpid;
    syscall[SYSCALL_YIELD] = (long(*)())scheduler;
    syscall[SYSCALL_TASKSET] = (long(*)())do_taskset;
    syscall[SYSCALL_SHOW_EXEC] = (long(*)())do_show_exec;
    syscall[SYSCALL_FUTEX_WAIT] = (long(*)())futex_wait;
    syscall[SYSCALL_FUTEX_WAKEUP] = (long(*)())futex_wakeup;
    syscall[SYSCALL_BINSEM_GET] = (long(*)())binsem_get;
    syscall[SYSCALL_BINSEM_OP] = (long(*)())binsem_op;
    syscall[SYSCALL_SHMPGET] = (long(*)())shm_page_get;
    syscall[SYSCALL_SHMPDT] = (long(*)())shm_page_dt;
    syscall[SYSCALL_WRITE] = (long(*)())screen_write;
    syscall[SYSCALL_CURSOR] = (long(*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH] = (long(*)())screen_reflush;
    syscall[SYSCALL_SCREEN_CLEAR] = (long(*)())screen_clear;
    syscall[SYSCALL_SCREEN_SCROLL] = (long(*)())screen_scroll;
    syscall[SYSCALL_GET_TIMEBASE] = (long(*)())get_time_base;
    syscall[SYSCALL_GET_TICK] = (long(*)())get_ticks;   
    syscall[SYSCALL_GET_CHAR] = (long(*)())sbi_console_getchar; 
    syscall[SYSCALL_MAILBOX_OPEN] = (long(*)())do_mbox_open;   
    syscall[SYSCALL_MAILBOX_CLOSE] = (long(*)())do_mbox_close; 
    syscall[SYSCALL_MAILBOX_SEND] = (long(*)())do_mbox_send;  
    syscall[SYSCALL_MAILBOX_RECV] = (long(*)())do_mbox_recv;  
}

// jump from start.S
// The beginning of everything
int main()
{   
    // read CPU frequency and calc timer interval
    time_base = sbi_read_fdt(TIMEBASE);
    timer_interval = (uint64_t)(time_base / 100);

    // init Process Control Block
    init_pcb();
    printk("> [INIT] PCB Initialization Succeeded.\n\r");

    // init futex mechanism and binsem mechanism
    init_system_futex();
    init_system_binsem();

    // init interrupt
    init_exception();
    printk("> [INIT] Interrupt Processing Initialization Succeeded.\n\r");

    // init system call table
    init_syscall();
    printk("> [INIT] System Call Initialized Successfully.\n\r");

    // init screen
    init_screen();
    printk("> [INIT] Screen Initialization Succeeded.\n\r");

    // wakeup another core
    spin_lock_init(&kernel_lock);
    wakeup_other_hart();
    while(!smp_init_flag){
        ;
    }
    printk("> [INIT] Start the Second Core Successfully.\n\r");
    
    // Setup timer interrupt and enable all interrupt
    enable_interrupt();
    setup_exception();
    sbi_set_timer(get_ticks() + timer_interval);

    while (1){
        disable_preempt();
        list_node_t* clean_node = kernel_pcb[0].wait_list.next;
        while(clean_node != &kernel_pcb[0].wait_list){
            pcb_t *clean_pcb = list_entry(clean_node, pcb_t, list);
            // release pcb
            clean_pcb->status = TASK_EXITED;
            clean_pcb->pid = -1;
            // release kernel stack
            free_user_page(kva2pa(clean_pcb->kernel_stack_base - PAGE_SIZE));
            
            clean_node = clean_node->next;
            list_del(clean_node->prev);
        }
    
        enable_preempt();
        __asm__ __volatile__("wfi\n\r":::);
    }
}