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
    .type = KERNEL_THREAD,
    .status = TASK_RUNNING
};

LIST_HEAD(ready_queue);
LIST_HEAD(blocked_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    if(!list_empty(&ready_queue)){
        // Modify the current_running pointer, modify the ready queue
        pcb_t* prev_running = current_running;
        current_running = list_entry(ready_queue.prev, pcb_t, list);
        list_move(ready_queue.prev, &ready_queue);
        process_id = current_running->pid;

        current_running->status = TASK_RUNNING;
        prev_running->status = TASK_READY;

        // restore the current_running's cursor_x and cursor_y
        vt100_move_cursor(current_running->cursor_x,
                        current_running->cursor_y);
        screen_cursor_x = current_running->cursor_x;
        screen_cursor_y = current_running->cursor_y;

        // TODO: switch_to current_running
        switch_to(prev_running, current_running);
    }
    else{
        __asm__ __volatile__("wfi\n\r":::);
    }
}

void do_sleep(uint32_t sleep_time)
{
    // TODO: sleep(seconds)
    // note: you can assume: 1 second = `timebase` ticks
    // 1. block the current_running
    // 2. create a timer which calls `do_unblock` when timeout
    // 3. reschedule because the current_running is blocked.
    list_node_t* parameter = &(current_running->list);
    timer_create(do_unblock, (void*)parameter, get_ticks() + time_base * sleep_time);
    do_block(&(current_running->list), &blocked_queue);
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // block the pcb task into the block queue
    list_del(pcb_node);
    list_add(pcb_node, queue);
    list_entry(pcb_node, pcb_t, list)->status = TASK_BLOCKED;
    if(&(current_running ->list) == pcb_node){
        do_scheduler();
    }
}

void do_unblock(list_node_t *pcb_node)
{
    // unblock the `pcb` from the block queue
    list_del(pcb_node);
    list_add(pcb_node, &ready_queue);
    list_entry(pcb_node, pcb_t, list)->status = TASK_READY;
}