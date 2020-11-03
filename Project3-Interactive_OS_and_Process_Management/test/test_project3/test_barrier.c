#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <mthread.h>
#include <test3.h>

#define NUM_TB 3

static mthread_barrier_t barrier;

void test_barrier(void)
{
    srand(clock());

    mthread_barrier_init(&barrier, NUM_TB);

    struct task_info child_task = {(uintptr_t)&barrier_task,
                                   USER_PROCESS};
    pid_t pids[NUM_TB];
    for (int i = 0; i < NUM_TB; ++i) {
        pids[i] = sys_spawn(&child_task, (void*)(i + 1),
                            ENTER_ZOMBIE_ON_EXIT);
    }

    for (int i = 0; i < NUM_TB; ++i) {
        sys_waitpid(pids[i]);
    }

    mthread_barrier_destroy(&barrier);

    sys_exit();
}

void barrier_task(int print_location)
{
    int i;

    for (i = 0; i < 10; i++)
    {
        int next = rand() % 3 + 1;
        sys_move_cursor(1, print_location);
        printf("> [TASK] Ready to enter the barrier.(%d)", i);

        mthread_barrier_wait(&barrier);

        sys_move_cursor(1, print_location);
        printf("> [TASK] Exited barrier (%d).(sleep %d s)",
               i, next);
        sys_sleep(next);
    }

    sys_exit();
}
