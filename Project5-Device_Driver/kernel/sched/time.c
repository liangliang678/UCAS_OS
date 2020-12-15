#include <os/time.h>
#include <os/mm.h>
#include <os/irq.h>
#include <os/smp.h>
#include <type.h>

LIST_HEAD(timer_queue);

uint64_t time_elapsed = 0;
uint32_t time_base = 0;
uint64_t timer_interval = 0;

void timer_create(TimerCallback func, void* parameter, uint64_t tick)
{
    disable_preempt();

    list_add_tail(&current_running[cpu_id]->timer.list, &timer_queue);
    current_running[cpu_id]->timer.timeout_tick = tick;
    current_running[cpu_id]->timer.callback_func = func;
    current_running[cpu_id]->timer.parameter = parameter;

    enable_preempt();
}

/* check all timers. if timeouts, call callback_func and free the timer */
void timer_check()
{
    disable_preempt();

    uint64_t current_tick = get_ticks();
    list_node_t* p = timer_queue.next;

    while(p != &timer_queue){
        timer_t *timer_node = list_entry(p, timer_t, list);
        if(current_tick >= timer_node->timeout_tick){
            (timer_node->callback_func)(timer_node->parameter);
            p = p->next;
            list_del(p->prev);
        }
        else{
            p = p->next;
        } 
    }

    enable_preempt();
}

uint64_t get_ticks()
{
    __asm__ __volatile__(
        "rdtime %0"
        : "=r"(time_elapsed));
    return time_elapsed;
}

uint64_t get_timer()
{
    return get_ticks() / time_base;
}

uint64_t get_time_base()
{
    return time_base;
}

void latency(uint64_t time)
{
    uint64_t begin_time = get_timer();

    while (get_timer() - begin_time < time);
    return;
}