#include <stdatomic.h>
#include <stdint.h>
#include <mthread.h>
#include <assert.h>
#include <sys/syscall.h>

int mthread_spin_init(mthread_spinlock_t *lock)
{
    // TODO:
}
int mthread_spin_destroy(mthread_spinlock_t *lock) {
    // TODO:
}
int mthread_spin_trylock(mthread_spinlock_t *lock)
{
    // TODO:
}
int mthread_spin_lock(mthread_spinlock_t *lock)
{
    // TODO:
}
int mthread_spin_unlock(mthread_spinlock_t *lock)
{
    // TODO:
}

int mthread_mutex_init(mthread_mutex_t *lock)
{
    // TODO:
}
int mthread_mutex_destroy(mthread_mutex_t *lock) {
    // TODO:
}
int mthread_mutex_trylock(mthread_mutex_t *lock) {
    // TODO:
}
int mthread_mutex_lock(mthread_mutex_t *lock) {
    // TODO:
}
int mthread_mutex_unlock(mthread_mutex_t *lock)
{
    // TODO:
}

int mthread_barrier_init(mthread_barrier_t * barrier, unsigned count)
{
    // TODO:
}
int mthread_barrier_wait(mthread_barrier_t *barrier)
{
    // TODO:
}
int mthread_barrier_destroy(mthread_barrier_t *barrier)
{
    // TODO:
}

int mthread_cond_init(mthread_cond_t *cond)
{
    // TODO:
}
int mthread_cond_destroy(mthread_cond_t *cond) {
    // TODO:
}
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex)
{
    // TODO:
}
int mthread_cond_signal(mthread_cond_t *cond)
{
    // TODO:
}
int mthread_cond_broadcast(mthread_cond_t *cond)
{
    // TODO:
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