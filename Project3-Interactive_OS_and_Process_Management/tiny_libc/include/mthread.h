/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                              A Mini PThread-like library
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef MTHREAD_H_
#define MTHREAD_H_

#include <stdint.h>
#include <stdatomic.h>

/* on success, these functions return zero. Otherwise, return an error number */
#define EBUSY  1 /* the lock is busy */
#define EINVAL 2 /* the lock is invalid */

typedef atomic_long mthread_spinlock_t;
typedef atomic_long mthread_mutex_t;

typedef enum {
    INVALID,
    UNLOCKED,
    LOCKED,
} lock_status_t;

int mthread_spin_init(mthread_spinlock_t *lock);
int mthread_spin_destroy(mthread_spinlock_t *lock);
int mthread_spin_trylock(mthread_spinlock_t *lock);
int mthread_spin_lock(mthread_spinlock_t *lock);
int mthread_spin_unlock(mthread_spinlock_t *lock);

int mthread_mutex_init(mthread_mutex_t *lock);
int mthread_mutex_destroy(mthread_mutex_t *lock);
int mthread_mutex_trylock(mthread_mutex_t *lock);
int mthread_mutex_lock(mthread_mutex_t *lock);
int mthread_mutex_unlock(mthread_mutex_t *lock);

typedef struct mthread_barrier
{
    int count;
    int reached;
    long futex;
} mthread_barrier_t;

int mthread_barrier_init(mthread_barrier_t * barrier, unsigned count);
int mthread_barrier_wait(mthread_barrier_t *barrier);
int mthread_barrier_destroy(mthread_barrier_t *barrier);

typedef atomic_int mthread_cond_t;

int mthread_cond_init(mthread_cond_t *cond);
int mthread_cond_destroy(mthread_cond_t *cond);
int mthread_cond_wait(mthread_cond_t *cond, mthread_mutex_t *mutex);
int mthread_cond_signal(mthread_cond_t *cond);
int mthread_cond_broadcast(mthread_cond_t *cond);

typedef struct mthread_semaphore
{
    // TODO:
} mthread_semaphore_t;

int mthread_semaphore_init(mthread_semaphore_t *sem, int val);
int mthread_semaphore_up(mthread_semaphore_t *sem);
int mthread_semaphore_down(mthread_semaphore_t *sem);
int mthread_semaphore_destroy(mthread_semaphore_t *sem);

#endif
