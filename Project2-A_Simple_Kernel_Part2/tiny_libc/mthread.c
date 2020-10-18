#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>

int mthread_spin_init(mthread_spinlock_t *lock)
{
    /* TODO: */
    atomic_exchange(lock, UNLOCKED);
    return 0;
}
int mthread_spin_destroy(mthread_spinlock_t *lock) {
    // do nothing.
    return 0;
}
int mthread_spin_trylock(mthread_spinlock_t *lock)
{
    /* TODO: return 0 or EBUSY */
    if(atomic_compare_exchange(lock, UNLOCKED, LOCKED)!=LOCKED){
        return 0;
    }
    else return EBUSY;
}
int mthread_spin_lock(mthread_spinlock_t *lock)
{
    /* TODO: */
    while(mthread_spin_trylock(lock)){
        ;
    }
}
int mthread_spin_unlock(mthread_spinlock_t *lock)
{
    /* TODO: */
    atomic_exchange(lock, UNLOCKED);
    return 0;
}

int mthread_mutex_init(mthread_mutex_t *lock)
{
    /* TODO: */
    atomic_exchange_d(lock, UNLOCKED);
    return 0;
}
int mthread_mutex_destroy(mthread_mutex_t *lock) {
    // do nothing!
    return 0;
}
int mthread_mutex_trylock(mthread_mutex_t *lock) {
    /* TODO: */
    if(atomic_compare_exchange_d(lock, UNLOCKED, LOCKED)!=LOCKED){
        return 0;
    }
    else return EBUSY;
}
int mthread_mutex_lock(mthread_mutex_t *lock) {
    /* TODO: */
    while(mthread_mutex_trylock(lock)){
        sys_futex_wait(lock, LOCKED);
    }
    return 0;
}
int mthread_mutex_unlock(mthread_mutex_t *lock)
{
    /* TODO: */
    atomic_exchange_d(lock, UNLOCKED);
    sys_futex_wakeup(lock, 1);
    return 0;
}
