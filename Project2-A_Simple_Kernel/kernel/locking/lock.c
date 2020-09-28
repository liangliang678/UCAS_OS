#include <os/lock.h>
#include <os/sched.h>
#include <atomic.h>

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO */
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO */
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO */
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO */
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
    /* TODO */
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
    /* TODO */
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
    /* TODO */
}
