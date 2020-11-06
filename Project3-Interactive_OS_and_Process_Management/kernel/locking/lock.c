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