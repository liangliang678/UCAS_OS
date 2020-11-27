#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <os/binsem.h>
#include <os/smp.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>
#include <csr.h>
#include <user_programs.h>
#include <os/elf.h>

extern void __global_pointer$();

pcb_t pcb[NUM_MAX_TASK];
pcb_t kernel_pcb[NR_CPUS];
const pcb_t *core0_tp = &kernel_pcb[0];
const pcb_t *core1_tp = &kernel_pcb[1];
const ptr_t kernel_stack_0 = INIT_KERNEL_STACK + PAGE_SIZE;
const ptr_t kernel_stack_1 = INIT_KERNEL_STACK + 2 * PAGE_SIZE;

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* current running task PCB */
pcb_t * volatile current_running[NR_CPUS];
pcb_t * volatile current_cpu_running;

/* global process id */
pid_t process_id = 2;

/* 
 * Only modify PCB, current_running, process_id and ready_queue
 * DO NOT STORE OR RESTORE 
 * trick: modify $tp in ret_from_exception to ensure
 * disable_preempt() and enable_preempt() modify the same PCB
 */
void scheduler(void)
{
    // store the current_running's cursor_x and cursor_y
    current_running[cpu_id]->cursor_x = screen_cursor_x[cpu_id];
    current_running[cpu_id]->cursor_y = screen_cursor_y[cpu_id];

    // Modify the current_running pointer and the ready queue
    if(current_running[cpu_id]->status == TASK_RUNNING){
        list_add_tail(&current_running[cpu_id]->list, &ready_queue);
        current_running[cpu_id]->status = TASK_READY;
        current_running[cpu_id]->ready_tick = get_ticks();
    }
    else if(current_running[cpu_id]->status == TASK_KILLED){
        // unblock tasks in wait queue
        list_node_t *wakeup_pcb = current_running[cpu_id]->wait_list.next;
        while(wakeup_pcb != &current_running[cpu_id]->wait_list){
            wakeup_pcb = wakeup_pcb->next;
            do_unblock(wakeup_pcb->prev);
        }

        // release binsem
        for(int i = 0; i < current_running[cpu_id]->binsem_num; i++){
            int binsem_id = current_running[cpu_id]->binsem_id[i];
            binsem_nodes[binsem_id].sem++;
            if(binsem_nodes[binsem_id].sem <= 0){
                list_node_t *unblocked_pcb_list = binsem_nodes[binsem_id].block_queue.next;
                pcb_t *unblocked_pcb = list_entry(unblocked_pcb_list, pcb_t, list);
                do_unblock(unblocked_pcb_list);
                unblocked_pcb->binsem_id[unblocked_pcb->binsem_num] = binsem_id;
                unblocked_pcb->binsem_num++;
            }
        }

        // release user stack
        freePage(current_running[cpu_id]->user_stack_base);

        // enter ZOMBIE status
        if(current_running[cpu_id]->mode == AUTO_CLEANUP_ON_EXIT){
            current_running[cpu_id]->status = TASK_ZOMBIE;
            list_add_tail(&current_running[cpu_id]->list, &kernel_pcb[0].wait_list);
        }
        else{
            current_running[cpu_id]->status = TASK_ZOMBIE;
        }
    }
    current_running[cpu_id] = list_entry(max_priority_node(), pcb_t, list); 
    list_del(&current_running[cpu_id]->list);
    current_running[cpu_id]->status = TASK_RUNNING;
    current_running[cpu_id]->cpu_id = cpu_id;
    current_cpu_running = current_running[cpu_id];
    
    set_satp(SATP_MODE_SV39, current_cpu_running->pid, (uintptr_t)(current_cpu_running->pgdir) >> NORMAL_PAGE_SHIFT);
    local_flush_tlb_all();
    // restore the current_running's cursor_x and cursor_y
    vt100_move_cursor(current_running[cpu_id]->cursor_x,
                      current_running[cpu_id]->cursor_y);
    screen_cursor_x[cpu_id] = current_running[cpu_id]->cursor_x;
    screen_cursor_y[cpu_id] = current_running[cpu_id]->cursor_y;
}

pid_t do_spawn(task_info_t *task, void* arg, spawn_mode_t mode)
{
    pcb_t *new_pcb = NULL;
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == -1){
            new_pcb = &pcb[i];
            break;
        }
    }
    if(!new_pcb){
        return -1;
    }
    
    new_pcb->kernel_sp = allocPage(1); 
    new_pcb->user_sp = allocPage(1);
    new_pcb->preempt_count = 0;
    new_pcb->kernel_stack_base = new_pcb->kernel_sp;
    new_pcb->user_stack_base = new_pcb->user_sp;
    list_add_tail(&new_pcb->list, &ready_queue);
    init_list_head(&new_pcb->wait_list);
    new_pcb->binsem_num = 0;
    new_pcb->pid = process_id++;
    new_pcb->type = task->type;
    new_pcb->status = TASK_READY;
    new_pcb->mode = mode;
    new_pcb->priority = 0;
    new_pcb->ready_tick = get_ticks();
    new_pcb->mask = current_running[cpu_id]->mask;

    /* initialization registers on kernel stack */
    regs_context_t *pt_regs = (regs_context_t *)(new_pcb->kernel_sp - sizeof(regs_context_t));
    new_pcb->kernel_sp -= sizeof(regs_context_t);
    pt_regs->regs[1] = (reg_t)task->entry_point;        //ra
    pt_regs->regs[2] = (reg_t)new_pcb->user_sp;         //sp
    pt_regs->regs[3] = (reg_t)__global_pointer$;        //gp
    pt_regs->regs[10]= (reg_t)arg;                      //a0
    pt_regs->sstatus = (reg_t)SR_SPIE;                  //SPP = 0, SPIE = 1
    pt_regs->sepc = (reg_t)task->entry_point;           //sepc = ra
    pt_regs->sscratch = (reg_t)new_pcb;                 //sscratch = tp

    return new_pcb->pid;
}

void do_exit()
{
    // unblock tasks in wait queue
    list_node_t *wakeup_pcb = current_running[cpu_id]->wait_list.next;
    while(wakeup_pcb != &current_running[cpu_id]->wait_list){
        wakeup_pcb = wakeup_pcb->next;
        do_unblock(wakeup_pcb->prev);
    }

    // release binsem
    for(int i = 0; i < current_running[cpu_id]->binsem_num; i++){
        int binsem_id = current_running[cpu_id]->binsem_id[i];
        binsem_nodes[binsem_id].sem++;
        if(binsem_nodes[binsem_id].sem <= 0){
            list_node_t *unblocked_pcb_list = binsem_nodes[binsem_id].block_queue.next;
            pcb_t *unblocked_pcb = list_entry(unblocked_pcb_list, pcb_t, list);
            do_unblock(unblocked_pcb_list);
            unblocked_pcb->binsem_id[unblocked_pcb->binsem_num] = binsem_id;
            unblocked_pcb->binsem_num++;
        }
    }

    // release user stack
    freePage(current_running[cpu_id]->user_stack_base);

    // delete from ready queue or blocked queue
    list_del(&current_running[cpu_id]->list);

    // enter ZOMBIE status
    if(current_running[cpu_id]->mode == AUTO_CLEANUP_ON_EXIT){
        current_running[cpu_id]->status = TASK_ZOMBIE;
        list_add_tail(&current_running[cpu_id]->list, &kernel_pcb[0].wait_list);
    }
    else{
        current_running[cpu_id]->status = TASK_ZOMBIE;
    }

    scheduler();
}

// sleep(seconds)
void do_sleep(uint32_t sleep_time)
{
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked
    do_block(&current_running[cpu_id]->list, &sleep_queue);
    list_node_t* parameter = &current_running[cpu_id]->list;
    timer_create((TimerCallback)do_unblock, (void*)parameter, get_ticks() + time_base * sleep_time);
    scheduler();
}

int do_kill(pid_t pid)
{
    // search the pcb
    pcb_t *killed_pcb = NULL;
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            killed_pcb = &pcb[i];
            break;
        }
    }
    if(killed_pcb == NULL){
        return 0;
    }

    if(killed_pcb->status == TASK_RUNNING){
        killed_pcb->status = TASK_KILLED;
        return 1;
    }

    // unblock tasks in wait queue
    list_node_t *wakeup_pcb = killed_pcb->wait_list.next;
    while(wakeup_pcb != &killed_pcb->wait_list){
        wakeup_pcb = wakeup_pcb->next;
        do_unblock(wakeup_pcb->prev);   
    }

    // release binsem mutex
    for(int i = 0; i < killed_pcb->binsem_num; i++){
        int binsem_id = killed_pcb->binsem_id[i];
        binsem_nodes[binsem_id].sem++;
        if(binsem_nodes[binsem_id].sem <= 0){
            list_node_t * unblocked_pcb_list = binsem_nodes[binsem_id].block_queue.next;
            pcb_t *unblocked_pcb = list_entry(unblocked_pcb_list, pcb_t, list);
            do_unblock(unblocked_pcb_list);
            unblocked_pcb->binsem_id[unblocked_pcb->binsem_num] = binsem_id;
            unblocked_pcb->binsem_num++;
        }
    }

    // release user stack
    freePage(killed_pcb->user_stack_base);

    // delete from ready queue or blocked queue
    list_del(&killed_pcb->list);

    // delete timer
    list_node_t *p = timer_queue.next;
    while(p != &timer_queue){
        timer_t *timer_node = list_entry(p, timer_t, list);
        pcb_t *pcb_node = list_entry(timer_node, pcb_t, timer);
        if(pcb_node == killed_pcb){
            p = p->next;
            list_del(p->prev);
        }
        else{
            p = p->next;
        } 
    }

    // enter ZOMBIE status
    if(killed_pcb->mode == AUTO_CLEANUP_ON_EXIT){
        killed_pcb->status = TASK_ZOMBIE;
        list_add_tail(&killed_pcb->list, &kernel_pcb[0].wait_list);
    }
    else{
        killed_pcb->status = TASK_ZOMBIE;
    }

    scheduler();
    return 1;
}

int do_waitpid(pid_t pid, reg_t ignore1, reg_t ignore2, regs_context_t *regs)
{
    pcb_t *child_pcb = NULL;
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            child_pcb = &pcb[i];
            break;
        }
    }
    if(child_pcb == NULL){
        return 0;
    }
    
    child_pcb->mode = ENTER_ZOMBIE_ON_EXIT;
    if(child_pcb->status == TASK_ZOMBIE){
        // release pcb
        child_pcb->status = TASK_EXITED;
        child_pcb->pid = -1;
        // release kernel stack
        freePage(child_pcb->kernel_stack_base);

        return 1;
    }
    else{
        // if child task has not exited, call do_waitpid again when parent task is unblocked
        // trick: return pid to keep a0 unchanged
        regs->sepc = regs->sepc - 4;
        do_block(&current_running[cpu_id]->list, &child_pcb->wait_list);
        scheduler();
        return pid;
    }   
}

void do_process_show(char* buffer)
{
    buffer = get_kva_of(buffer, current_running[cpu_id]->pgdir);
    buffer[0] = '\0';
    strcat(buffer, "[PROCESS TABLE]\n");
    strcat(buffer, "INIT 0    STATUS: ");
    if(kernel_pcb[0].status == TASK_BLOCKED){
        strcat(buffer, "BLOCKED    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[0].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[0].status == TASK_RUNNING){
        strcat(buffer, "RUNNING    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[0].mask);
        strcat(buffer, mask);
        strcat(buffer, "    CORE: ");
        char cpu_id[2];
        itoa(cpu_id, kernel_pcb[0].cpu_id);
        strcat(buffer, cpu_id);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[0].status == TASK_READY){
        strcat(buffer, "READY    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[0].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[0].status == TASK_EXITED){
        strcat(buffer, "EXITED    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[0].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[0].status == TASK_ZOMBIE){
        strcat(buffer, "ZOMBIE    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[0].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    
    strcat(buffer, "INIT 1    STATUS: ");
    if(kernel_pcb[1].status == TASK_BLOCKED){
        strcat(buffer, "BLOCKED\n");
    }
    else if(kernel_pcb[1].status == TASK_RUNNING){
        strcat(buffer, "RUNNING    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[1].mask);
        strcat(buffer, mask);
        strcat(buffer, "    CORE: ");
        char cpu_id[2];
        itoa(cpu_id, kernel_pcb[1].cpu_id);
        strcat(buffer, cpu_id);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[1].status == TASK_READY){
        strcat(buffer, "READY    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[1].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[1].status == TASK_EXITED){
        strcat(buffer, "EXITED    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[1].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }
    else if(kernel_pcb[1].status == TASK_ZOMBIE){
        strcat(buffer, "ZOMBIE    MASK: ");
        char mask[2];
        itoa(mask, kernel_pcb[1].mask);
        strcat(buffer, mask);
        strcat(buffer, "\n");
    }

    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid != -1){
            strcat(buffer, "PID: ");
            char pid[10];
            itoa(pid, pcb[i].pid);
            strcat(buffer, pid);
            strcat(buffer, "    STATUS: ");
            if(pcb[i].status == TASK_BLOCKED){
                strcat(buffer, "BLOCKED    MASK: ");
                char mask[2];
                itoa(mask, pcb[i].mask);
                strcat(buffer, mask);
                strcat(buffer, "\n");
            }
            else if(pcb[i].status == TASK_RUNNING){
                strcat(buffer, "RUNNING    MASK: ");
                char mask[2];
                itoa(mask, pcb[i].mask);
                strcat(buffer, mask);
                strcat(buffer, "    CORE: ");
                char cpu_id[2];
                itoa(cpu_id, pcb[i].cpu_id);
                strcat(buffer, cpu_id);
                strcat(buffer, "\n");
            }
            else if(pcb[i].status == TASK_READY){
                strcat(buffer, "READY    MASK: ");
                char mask[2];
                itoa(mask, pcb[i].mask);
                strcat(buffer, mask);
                strcat(buffer, "\n");
            }
            else if(pcb[i].status == TASK_EXITED){
                strcat(buffer, "EXITED    MASK: ");
                char mask[2];
                itoa(mask, pcb[i].mask);
                strcat(buffer, mask);
                strcat(buffer, "\n");
            }
            else if(pcb[i].status == TASK_ZOMBIE){
                strcat(buffer, "ZOMBIE    MASK: ");
                char mask[2];
                itoa(mask, pcb[i].mask);
                strcat(buffer, mask);
                strcat(buffer, "\n");
            }
        }
    }
}

pid_t do_getpid()
{
    return current_running[cpu_id]->pid;
}

int do_taskset(pid_t pid, unsigned long mask)
{
    // search the pcb
    pcb_t *set_pcb = NULL;
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            set_pcb = &pcb[i];
            break;
        }
    }
    if(set_pcb == NULL){
        return 0;
    }

    set_pcb->mask = mask;
    return 1;
}

pid_t do_exec(const char* file_name, int argc, char* argv[], spawn_mode_t mode)
{
    file_name = get_kva_of(file_name, current_running[cpu_id]->pgdir);
    argv = (char (*)[])get_kva_of(argv, current_running[cpu_id]->pgdir);
    char* _argv[9];
    for(int i = 0; i < argc; i++){
        _argv[i] = get_kva_of(argv[i], current_running[cpu_id]->pgdir);
    }
    
    char *binary;
    int length;
    if(!get_elf_file(file_name, &binary, &length)){
        return 0;
    }

    pcb_t *new_pcb = NULL;
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid == -1){
            new_pcb = &pcb[i];
            break;
        }
    }
    if(!new_pcb){
        return -1;
    }
    
    new_pcb->kernel_sp = pa2kva(allocPage() + PAGE_SIZE); 
    new_pcb->user_sp = USER_STACK_ADDR;
    new_pcb->preempt_count = 0;
    new_pcb->kernel_stack_base = new_pcb->kernel_sp;
    new_pcb->user_stack_base = new_pcb->user_sp;
    list_add_tail(&new_pcb->list, &ready_queue);
    init_list_head(&new_pcb->wait_list);
    new_pcb->binsem_num = 0;
    new_pcb->pid = process_id++;
    new_pcb->type = USER_PROCESS;
    new_pcb->status = TASK_READY;
    new_pcb->mode = mode;
    new_pcb->priority = 0;
    new_pcb->ready_tick = get_ticks();
    new_pcb->mask = current_running[cpu_id]->mask;
    new_pcb->pgdir = init_page_table();
    
    uintptr_t entry = load_elf(binary, length, new_pcb->pgdir, get_kva_of);

    /* initialization registers on kernel stack */
    regs_context_t *pt_regs = (regs_context_t *)(new_pcb->kernel_sp - sizeof(regs_context_t));
    new_pcb->kernel_sp -= sizeof(regs_context_t);
    pt_regs->regs[1] = (reg_t)entry;                    //ra
    pt_regs->regs[2] = (reg_t)new_pcb->user_sp;         //sp
    pt_regs->regs[3] = (reg_t)__global_pointer$;        //gp
    pt_regs->sstatus = (reg_t)SR_SPIE;                  //SPP = 0, SPIE = 1
    pt_regs->sepc = (reg_t)entry;                       //sepc = ra
    pt_regs->sscratch = (reg_t)new_pcb;                 //sscratch = tp

    /* pass arg to new process */
    pt_regs->regs[10] = (reg_t)argc;                     //a0
    new_pcb->user_sp -= 256;                             //sp = sp - 256
    pt_regs->regs[2]  = (reg_t)new_pcb->user_sp;         //sp
    pt_regs->regs[11] = (reg_t)new_pcb->user_sp;         //a1
    uintptr_t dest = new_pcb->user_sp + 72;
    for(int i = 0; i < argc; i++){
        uintptr_t pointer = get_kva_of(new_pcb->user_sp + 8 * i, new_pcb->pgdir);
        *(uint64_t*)pointer = dest;
        memcpy(get_kva_of(dest, new_pcb->pgdir), _argv[i], strlen(_argv[i]) + 1);
        dest += strlen(_argv[i]) + 1;
    }
    return new_pcb->pid;
}

void do_show_exec(char* buffer)
{
    buffer = get_kva_of(buffer, current_running[cpu_id]->pgdir);
    buffer[0] = '\0';
    strcat(buffer, "[EXEC LIST]\n");
    for(int i = 0; i < ELF_FILE_NUM; i++){
        strcat(buffer, elf_files[i].file_name);
        strcat(buffer, "\n");
    }
}
// block the pcb task into the block queue
void do_block(list_node_t *pcb_node, list_head *queue)
{
    list_del(pcb_node);
    list_add_tail(pcb_node, queue);
    list_entry(pcb_node, pcb_t, list)->status = TASK_BLOCKED;
}

// unblock the `pcb` from the block queue
void do_unblock(list_node_t *pcb_node)
{
    list_del(pcb_node);
    list_add_tail(pcb_node, &ready_queue);
    list_entry(pcb_node, pcb_t, list)->status = TASK_READY;
    list_entry(pcb_node, pcb_t, list)->ready_tick = get_ticks();
}

list_node_t* max_priority_node(void)
{
    int mask_flag;
    uint64_t current_tick = get_ticks();

    // max_priority_node has min points
    list_node_t* node;
    list_node_t* max_priority_node = NULL;
    int min_points = INT32_MAX;
    for(node = ready_queue.next; node != &ready_queue; node = node->next){
        pcb_t *node_pcb = list_entry(node, pcb_t, list);
        mask_flag = (cpu_id == 0 && ((node_pcb->mask & 0x1) == 0x1)) || (cpu_id == 1 && ((node_pcb->mask & 0x2) == 0x2));
        if(!mask_flag){
            continue;
        }
        int node_points = node_pcb->priority - (current_tick - node_pcb->ready_tick)/timer_interval;
        if(node_points < min_points){
            max_priority_node = node;
            min_points = node_points;
        }
    }
    assert(max_priority_node);
    return max_priority_node;
}