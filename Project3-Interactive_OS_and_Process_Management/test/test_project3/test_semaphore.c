#include <mthread.h>
#include <test3.h>
#include <time.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/syscall.h>

static mthread_semaphore_t semaphore;
static int global_count = 0;

void semaphore_add_task1(void)
{
    int i;
    int print_location = 1;
    // int sum_up = 0;

    mthread_semaphore_init(&semaphore, 1);

    struct task_info subtask1 = {(uintptr_t)&semaphore_add_task2, USER_PROCESS};
    struct task_info subtask2 = {(uintptr_t)&semaphore_add_task3, USER_PROCESS};
    sys_spawn(&subtask1, NULL, AUTO_CLEANUP_ON_EXIT);
    sys_spawn(&subtask2, NULL, AUTO_CLEANUP_ON_EXIT);

    for (i = 0; i < 10; i++)
    {
        mthread_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        mthread_semaphore_up(&semaphore);

        sys_move_cursor(1, print_location);
        printf("> [TASK] current global value %d. (%d)", global_count, i + 1);

        sys_sleep(1);
    }

    mthread_semaphore_destroy(&semaphore);

    sys_exit();
}

void semaphore_add_task2(void)
{
    int i;
    int print_location = 2;
    // int sum_down = 0;

    for (i = 0; i < 20; i++)
    {
        mthread_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        mthread_semaphore_up(&semaphore);

        sys_move_cursor(1, print_location);
        printf("> [TASK] current global value %d. (%d)", global_count, i + 1);

        sys_sleep(1);
    }

    sys_exit();
}

void semaphore_add_task3(void)
{
    int i;
    int print_location = 3;
    // int sum_down = 0;

    for (i = 0; i < 30; i++)
    {
        mthread_semaphore_down(&semaphore); // semaphore.value--
        // semaphore = 0
        global_count++;
        mthread_semaphore_up(&semaphore);

        sys_move_cursor(1, print_location);
        printf("> [TASK] current global value %d. (%d)", global_count, i + 1);

        sys_sleep(1);
    }

    sys_exit();
}
