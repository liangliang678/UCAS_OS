#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

void spin_lock_init(spin_lock_t *lock)
{
    atomic_swap(UNLOCKED, (ptr_t)(&lock->status));
}
void spin_lock_try_acquire(spin_lock_t *lock)
{
    while(atomic_cmpxchg(UNLOCKED, LOCKED, (ptr_t)(&lock->status)) == LOCKED){
        ;
    }
}
void spin_lock_acquire(spin_lock_t *lock)
{
    spin_lock_try_acquire(lock);
}
void spin_lock_release(spin_lock_t *lock)
{
    atomic_swap(UNLOCKED, (ptr_t)(&lock->status));
}