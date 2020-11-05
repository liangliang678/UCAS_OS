#include <stdatomic.h>
#include <stdint.h>
#include <mthread.h>
#include <assert.h>
#include <sys/syscall.h>

int mthread_spin_init(mthread_spinlock_t *lock)
{
    atomic_exchange_d(lock, UNLOCKED);
    return 0;
}
int mthread_spin_destroy(mthread_spinlock_t *lock) {
    atomic_exchange_d(lock, INVALID);
    return 0;
}
int mthread_spin_trylock(mthread_spinlock_t *lock)
{
    if(atomic_compare_exchange_d(lock, UNLOCKED, LOCKED) == UNLOCKED){
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
    atomic_exchange_d(lock, UNLOCKED);
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

int mthread_barrier_init(mthread_barrier_t * barrier, unsigned count)
{
    // TODO:
    barrier->count = count;
    barrier->reached = 0;
}
int mthread_barrier_wait(mthread_barrier_t *barrier)
{
    // TODO:
    (barrier->reached)++;
    if((barrier->reached) == (barrier->count)){
        barrier->reached = 0;
        sys_futex_wakeup(&barrier->futex, barrier->count - 1);        
    }
    else{      
        sys_futex_wait(&barrier->futex, barrier->futex);
    }
}
int mthread_barrier_destroy(mthread_barrier_t *barrier)
{
    // TODO:
}

int mthread_cond_init(mthread_cond_t *cond)
{
    // TODO:
    *cond = 0;
}
int mthread_cond_destroy(mthread_cond_t *cond) 
{
    // TODO:
    sys_futex_wakeup(cond, *cond);
}
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex)
{
    (*cond)++;
    mthread_mutex_unlock(mutex);
    sys_futex_wait(cond, *cond);
}
int mthread_cond_signal(mthread_cond_t *cond)
{
    // TODO:
    sys_futex_wakeup(cond, 1);
    (*cond)--;
}
int mthread_cond_broadcast(mthread_cond_t *cond)
{
    // TODO:
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