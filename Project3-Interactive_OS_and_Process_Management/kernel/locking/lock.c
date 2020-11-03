#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

void spin_lock_init(spin_lock_t *lock)
{
    atomic_swap_d(UNLOCKED, (ptr_t)(&lock->status));
}
void spin_lock_try_acquire(spin_lock_t *lock)
{
    while(lock->status == LOCKED){
        ;
    }
}
void spin_lock_acquire(spin_lock_t *lock)
{
    spin_lock_try_acquire(lock);
    atomic_swap_d(LOCKED, (ptr_t)(&lock->status));
}
void spin_lock_release(spin_lock_t *lock)
{
    atomic_swap_d(UNLOCKED, (ptr_t)(&lock->status));
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
    lock->lock.status = UNLOCKED;
    init_list_head(&lock->block_queue);
}
void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    while(lock->lock.status == LOCKED){
        do_block(&current_running->list, &lock->block_queue);
        do_scheduler();
    }
    lock->lock.status = LOCKED;
}
void do_mutex_lock_release(mutex_lock_t *lock)
{
    lock->lock.status = UNLOCKED;
    if(!list_empty(&lock->block_queue)){
        do_unblock(lock->block_queue.next);
        do_scheduler();
    }   
}