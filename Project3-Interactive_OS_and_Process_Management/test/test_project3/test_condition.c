#include <time.h>
#include <stdio.h>
#include <test3.h>
#include <sys/syscall.h>
#include <mthread.h>
#include <stdlib.h>

#define NUM_CONSUMER 3

static mthread_mutex_t mutex;
static mthread_cond_t condition;
static int num_staff = 0;

void test_condition(void)
{
    srand(clock());

    mthread_mutex_init(&mutex);
    mthread_cond_init(&condition);

    struct task_info task_p = {(uintptr_t)&producer_task,
                              USER_PROCESS};
    struct task_info task_c = {(uintptr_t)&consumer_task,
                              USER_PROCESS};
    pid_t pids[NUM_CONSUMER + 1];
    pids[0] = sys_spawn(&task_p, NULL,
                        ENTER_ZOMBIE_ON_EXIT);
    for (int i = 1; i <= NUM_CONSUMER; ++i) {
        pids[i] = sys_spawn(&task_c, (void*)(i + 1),
                            ENTER_ZOMBIE_ON_EXIT);
    }

    for (int i = 0; i < NUM_CONSUMER + 1; ++i) {
        sys_waitpid(pids[i]);
    }

    mthread_cond_destroy(&condition);
    mthread_mutex_destroy(&mutex);

    sys_exit();
}

void producer_task(void)
{
    int i;
    int print_location = 1;
    int production = 3;
    int sum_production = 0;

    for (i = 0; i < 10; i++)
    {
        mthread_mutex_lock(&mutex);

        num_staff += production;
        sum_production += production;

        mthread_mutex_unlock(&mutex);

        sys_move_cursor(1, print_location);
        int next = rand() % 5;
        printf("> [TASK] Total produced %d products. (next in %d seconds)", sum_production, next);

        // condition_signal(&condition);
        mthread_cond_broadcast(&condition);

        sys_sleep(next);
    }

    sys_exit();
}

void consumer_task(int print_location)
{
    int consumption = 1;
    int sum_consumption = 0;

    while (1)
    {
        mthread_mutex_lock(&mutex);

        while (num_staff == 0)
        {
            mthread_cond_wait(&condition, &mutex);
        }

        num_staff -= consumption;
        sum_consumption += consumption;

        int next = rand() % 3;
        sys_move_cursor(1, print_location);
        printf("> [TASK] Total consumed %d products. (Sleep %d seconds)", sum_consumption, next);

        mthread_mutex_unlock(&mutex);
        sys_sleep(next);
    }
}
