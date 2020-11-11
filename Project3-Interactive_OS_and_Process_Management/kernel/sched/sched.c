#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <os/binsem.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>
#include <csr.h>

extern void __global_pointer$();

pcb_t pcb[NUM_MAX_TASK];
const ptr_t kernel_stack_1 = INIT_KERNEL_STACK + PAGE_SIZE;
const ptr_t kernel_stack_2 = INIT_KERNEL_STACK + 2 * PAGE_SIZE;
pcb_t kernel_pcb[NR_CPUS];

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* current running task PCB */
pcb_t * volatile current_running;

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
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;

    // Modify the current_running pointer and the ready queue
    if(current_running->status == TASK_RUNNING){
        list_add_tail(&current_running->list, &ready_queue);
        current_running->status = TASK_READY;
        current_running->ready_tick = get_ticks();
    }
    current_running = list_entry(max_priority_node(), pcb_t, list); 
    list_del(&current_running->list);
    current_running->status = TASK_RUNNING;
        
    // restore the current_running's cursor_x and cursor_y
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
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
    list_node_t *wakeup_pcb = current_running->wait_list.next;
    while(wakeup_pcb != &current_running->wait_list){
        wakeup_pcb = wakeup_pcb->next;
        do_unblock(wakeup_pcb->prev);
    }

    // release binsem
    for(int i = 0; i < current_running->binsem_num; i++){
        int binsem_id = current_running->binsem_id[i];
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
    freePage(current_running->user_stack_base, 1);

    // delete from ready queue
    list_del(&current_running->list);

    // enter ZOMBIE status
    if(current_running->mode == AUTO_CLEANUP_ON_EXIT){
        current_running->status = TASK_ZOMBIE;
        list_add_tail(&current_running->list, &kernel_pcb[0].wait_list);
    }
    else{
        current_running->status = TASK_ZOMBIE;
    }

    scheduler();
}

// sleep(seconds)
void do_sleep(uint32_t sleep_time)
{
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked
    do_block(&current_running->list, &sleep_queue);
    list_node_t* parameter = &current_running->list;
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
    freePage(killed_pcb->user_stack_base, 1);

    // delete from ready queue
    list_del(&killed_pcb->list);

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
    
    if(child_pcb->status == TASK_ZOMBIE){
        if(child_pcb->pid != -1){
            // release pcb
            child_pcb->status = TASK_EXITED;
            child_pcb->pid = -1;
            // release kernel stack
            freePage(child_pcb->kernel_stack_base, 1);
        }

        return 1;
    }
    else{
        // if child task has not exited, call do_waitpid again when parent task is unblocked
        // trick: return pid to keep a0 unchanged
        regs->sepc = regs->sepc - 4;
        do_block(&current_running->list, &child_pcb->wait_list);
        scheduler();
        return pid;
    }   
}

void do_process_show(char* buffer)
{
    buffer[0] = '\0';
    kstrcat(buffer, "[PROCESS TABLE]\n");
    kstrcat(buffer, "PID: 0    STATUS: ");
    kstrcat(buffer, "READY\n");
    for(int i = 0; i < NUM_MAX_TASK; i++){
        if(pcb[i].pid != -1){
            kstrcat(buffer, "PID: ");
            char pid[10];
            kitoa(pid, pcb[i].pid);
            kstrcat(buffer, pid);
            kstrcat(buffer, "    STATUS: ");
            if(pcb[i].status == TASK_BLOCKED){
                kstrcat(buffer, "BLOCKED\n");
            }
            else if(pcb[i].status == TASK_RUNNING){
                kstrcat(buffer, "RUNNING\n");
            }
            else if(pcb[i].status == TASK_READY){
                kstrcat(buffer, "READY\n");
            }
            else if(pcb[i].status == TASK_EXITED){
                kstrcat(buffer, "EXITED\n");
            }
            else if(pcb[i].status == TASK_ZOMBIE){
                kstrcat(buffer, "ZOMBIE\n");
            }
        }
    }
    return buffer;
}

pid_t do_getpid()
{
    return current_running->pid;
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
    list_node_t* node;
    uint64_t current_tick = get_ticks();

    // max_priority_node has min points
    list_node_t* max_priority_node = ready_queue.next;
    pcb_t* node_pcb = list_entry(max_priority_node, pcb_t, list);
    int min_points = node_pcb->priority - (current_tick - node_pcb->ready_tick)/timer_interval;

    for(node = ready_queue.next->next; node != &ready_queue; node = node->next){
        node_pcb = list_entry(node, pcb_t, list);
        int node_points = node_pcb->priority - (current_tick - node_pcb->ready_tick)/timer_interval;
        if(node_points < min_points){
            max_priority_node = node;
            min_points = node_points;
        }       
    }

    return max_priority_node;
}