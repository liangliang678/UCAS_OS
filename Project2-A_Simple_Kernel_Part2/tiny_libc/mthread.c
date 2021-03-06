#include <stdatomic.h>
#include <mthread.h>
#include <sys/syscall.h>

int mthread_spin_init(mthread_spinlock_t *lock)
{
    atomic_exchange(lock, UNLOCKED);
    return 0;
}
int mthread_spin_destroy(mthread_spinlock_t *lock) {
    atomic_exchange(lock, INVALID);
    return 0;
}
int mthread_spin_trylock(mthread_spinlock_t *lock)
{
    if(atomic_compare_exchange(lock, UNLOCKED, LOCKED) == UNLOCKED){
        return 0;
    }
    else return EBUSY;
}
int mthread_spin_lock(mthread_spinlock_t *lock)
{
    while(mthread_spin_trylock(lock) == EBUSY){
        ;
    }
    return 0;
}
int mthread_spin_unlock(mthread_spinlock_t *lock)
{
    atomic_exchange(lock, UNLOCKED);
    return 0;
}

int mthread_mutex_init(mthread_mutex_t *lock)
{
    atomic_exchange_d(lock, UNLOCKED);
    return 0;
}
int mthread_mutex_destroy(mthread_mutex_t *lock) {
    atomic_exchange_d(lock, INVALID);
    return 0;
}
int mthread_mutex_trylock(mthread_mutex_t *lock) {
    if(atomic_compare_exchange_d(lock, UNLOCKED, LOCKED) == UNLOCKED){
        return 0;
    }
    else return EBUSY;
}
int mthread_mutex_lock(mthread_mutex_t *lock) {
    while(mthread_mutex_trylock(lock) == EBUSY){
        sys_futex_wait((volatile uint64_t*)lock, LOCKED);
    }
    return 0;
}
int mthread_mutex_unlock(mthread_mutex_t *lock)
{
    atomic_exchange_d(lock, UNLOCKED);
    sys_futex_wakeup((volatile uint64_t*)lock, 1);
    return 0;
}