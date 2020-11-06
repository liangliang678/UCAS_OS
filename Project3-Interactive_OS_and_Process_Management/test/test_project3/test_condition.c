#include <time.h>
#include <stdio.h>
#include <test3.h>
#include <sys/syscall.h>
#include <mthread.h>
#include <stdlib.h>

#define NUM_CONSUMER 3
#define LOCK_BINSEM_KEY 42

static mthread_cond_t condition;
static int num_staff = 0;

void test_condition(void)
{
    srand(clock());
    mthread_cond_init(&condition);

    struct task_info task_p = {(uintptr_t)&producer_task, USER_PROCESS};
    struct task_info task_c = {(uintptr_t)&consumer_task, USER_PROCESS};
    pid_t pids[NUM_CONSUMER + 1];
    pids[0] = sys_spawn(&task_p, NULL, ENTER_ZOMBIE_ON_EXIT);
    for (int i = 1; i <= NUM_CONSUMER; ++i) {
        pids[i] = sys_spawn(&task_c, (void*)(i + 1), ENTER_ZOMBIE_ON_EXIT);
    }

    for (int i = 0; i < NUM_CONSUMER + 1; ++i) {
        sys_waitpid(pids[i]);
    }

    mthread_cond_destroy(&condition);
    sys_exit();
}

void producer_task(void)
{
    int i;
    int print_location = 1;
    int production = 3;
    int sum_production = 0;
    int lock = sys_binsem_get(LOCK_BINSEM_KEY);

    for (i = 0; i < 10; i++)
    {
        sys_binsem_op(lock, BINSEM_OP_LOCK);

        num_staff += production;
        sum_production += production;

        sys_binsem_op(lock, BINSEM_OP_UNLOCK);

        sys_move_cursor(1, print_location);
        int next = rand() % 5;
        printf("> [TASK] Total produced %d products. (next in %d seconds)", sum_production, next);

        mthread_cond_signal(&condition);
        //mthread_cond_broadcast(&condition);

        sys_sleep(next);
    }

    sys_exit();
}

void consumer_task(int print_location)
{
    int consumption = 1;
    int sum_consumption = 0;
    int lock = sys_binsem_get(LOCK_BINSEM_KEY);

    while (1)
    {
        sys_binsem_op(lock, BINSEM_OP_LOCK);

        while (num_staff == 0)
        {
            mthread_cond_wait(&condition, lock);
        }

        num_staff -= consumption;
        sum_consumption += consumption;

        int next = rand() % 3;
        sys_move_cursor(1, print_location);
        printf("> [TASK] Total consumed %d products. (Sleep %d seconds)", sum_consumption, next);

        sys_binsem_op(lock, BINSEM_OP_UNLOCK);
        sys_sleep(next);
    }
}
