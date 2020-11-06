#include <stdatomic.h>
#include <stdint.h>
#include <mthread.h>
#include <assert.h>
#include <sys/syscall.h>

void mthread_barrier_init(mthread_barrier_t * barrier, unsigned count)
{
    barrier->count = count;
    barrier->reached = 0;
}
void mthread_barrier_wait(mthread_barrier_t *barrier)
{
    (barrier->reached)++;
    if((barrier->reached) == (barrier->count)){
        sys_futex_wakeup(barrier, barrier->count);
        barrier->reached = 0;            
    }
    else{      
        sys_futex_wait(barrier);
    }
}
void mthread_barrier_destroy(mthread_barrier_t *barrier)
{
    barrier->count = -1;
    barrier->reached = -1;
}

int mthread_cond_init(mthread_cond_t *cond)
{
    *cond = 0;
}
int mthread_cond_destroy(mthread_cond_t *cond) 
{
    sys_futex_wakeup(cond, *cond);
    *cond = -1;
}
int mthread_cond_wait(mthread_cond_t *cond, int binsem_id)
{
    (*cond)++;
    sys_binsem_op(binsem_id, BINSEM_OP_UNLOCK);
    sys_futex_wait(cond);
}
int mthread_cond_signal(mthread_cond_t *cond)
{
    sys_futex_wakeup(cond, 1);
    (*cond)--;
}
int mthread_cond_broadcast(mthread_cond_t *cond)
{
    sys_futex_wakeup(cond, *cond);
    *cond = 0;
}

int mthread_semaphore_init(mthread_semaphore_t *sem, int val)
{
    // TODO:
}
int mthread_semaphore_up(mthread_semaphore_t *sem)
{
    // TODO:
}
int mthread_semaphore_down(mthread_semaphore_t *sem)
{
    // TODO:
}
int mthread_semaphore_destroy(mthread_semaphore_t *sem)
{
    // TODO:
}