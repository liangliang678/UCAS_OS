#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO */
    atomic_swap_d(UNLOCKED, &(lock->status));
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO */
    int try_times;
    for(try_times = 0; try_times < 1000; try_times++){
        if(lock->status == UNLOCKED){
            break;
        }
    }

    if(lock->status == UNLOCKED){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO */
    while(!spin_lock_try_acquire(lock)){
        do_scheduler();
    }
    atomic_swap_d(LOCKED, &(lock->status));
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO */
    atomic_swap_d(UNLOCKED, &(lock->status));
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TODO */
    lock->lock.status = UNLOCKED;
    init_list_head(&(lock->block_queue));
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TODO */
    while(lock->lock.status == LOCKED){
        do_block(&(current_running->list), &(lock->block_queue));
    }
    lock->lock.status = LOCKED;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TODO */
    lock->lock.status = UNLOCKED;
    if(!list_empty(&(lock->block_queue)))
    {
        do_unblock(lock->block_queue.prev);
    }
}
