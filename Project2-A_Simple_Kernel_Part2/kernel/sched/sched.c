#include <os/list.h>
#include <os/mm.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/irq.h>
#include <screen.h>
#include <stdio.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack,
    .preempt_count = 0,
    .pid = 0,
    .type = KERNEL_PROCESS,
    .status = TASK_RUNNING,
    .priority = 0
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

/* 
 * Only modify PCB, current_running, process_id and ready_queue
 * DO NOT STORE OR RESTORE 
 * trick: modify $tp in ret_from_exception to ensure
 * disable_preempt() and enable_preempt() modify the same PCB
 */
void scheduler(void)
{
	test_sched_time(BEGIN);
	
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
    process_id = current_running->pid;
        
    // restore the current_running's cursor_x and cursor_y
    vt100_move_cursor(current_running->cursor_x,
                      current_running->cursor_y);
    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;

	test_sched_time(END);
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
    list_node_t* node_p;
    uint64_t current_tick = get_ticks();

    // max_priority_node has min points
    list_node_t* max_priority_node = ready_queue.next;
    pcb_t* max_priority_pcb = list_entry(max_priority_node, pcb_t, list);
    int min_points = max_priority_pcb->priority - (current_tick - max_priority_pcb->ready_tick)/timer_interval;

    for(node_p = ready_queue.next->next; node_p != &ready_queue; node_p = node_p->next){
        pcb_t* pcb_p = list_entry(node_p, pcb_t, list);
        int node_p_points = pcb_p->priority - (current_tick - pcb_p->ready_tick)/timer_interval;
        if(node_p_points < min_points){
            max_priority_node = node_p;
            max_priority_pcb = pcb_p;
            min_points = node_p_points;
        }       
    }

    return max_priority_node;
}

uint64_t sched_used_time;
void test_sched_time(int op)
{
	int print_location = 13;
	if(op == BEGIN){
		sched_used_time = get_ticks();
	}
	else if(op == END){
		sched_used_time = get_ticks() - sched_used_time;
	    vt100_move_cursor(1, print_location);
	    printk("time_base:\t\t%d", time_base);
	    vt100_move_cursor(1, print_location + 1);
	    printk("timer_interval:\t\t%d", timer_interval);
	    vt100_move_cursor(1, print_location + 2);
	    printk("sched_used_time:\t%d", sched_used_time);
	}
}