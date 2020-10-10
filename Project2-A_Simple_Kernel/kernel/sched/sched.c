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
const ptr_t pid0_stack = INIT_USER_STACK + PAGE_SIZE;
const ptr_t pid0_kernel_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .kernel_sp = (ptr_t)pid0_kernel_stack - sizeof(regs_context_t),
    .user_sp = (ptr_t)pid0_stack,
    .preempt_count = 0,
    .pid = 0,
    .type = KERNEL_THREAD,
    .status = TASK_RUNNING
};

LIST_HEAD(ready_queue);

/* current running task PCB */
pcb_t * volatile current_running;

/* global process id */
pid_t process_id = 1;

void do_scheduler(void)
{
    if(!list_empty(&ready_queue)){
        // TODO schedule
        // Modify the current_running pointer.
        pcb_t* prev_running = current_running;
        current_running = list_entry(ready_queue.prev, pcb_t, list);

        list_move(ready_queue.prev, &ready_queue);
        process_id = current_running->pid;
        current_running->status = TASK_RUNNING;
        prev_running->status = TASK_READY;

        // restore the current_runnint's cursor_x and cursor_y
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
}

void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: block the pcb task into the block queue
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: unblock the `pcb` from the block queue
}
