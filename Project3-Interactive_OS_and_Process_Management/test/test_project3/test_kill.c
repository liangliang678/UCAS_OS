#include <time.h>
#include <test3.h>
#include <mthread.h>
#include <stdio.h>
#include <os.h>
#include <stddef.h>
#include <sys/syscall.h>

mthread_mutex_t lock1;
mthread_mutex_t lock2;

void ready_to_exit_task()
{
    int i = 0, print_location = 1;

    mthread_mutex_lock(&lock1);
    mthread_mutex_lock(&lock2);

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", sys_getpid(), i++);
    }

    mthread_mutex_unlock(&lock2);
    mthread_mutex_unlock(&lock1);
    sys_exit(); // test exit
}

void wait_lock_task(long other_pid)
{
    int print_location = 2;

    sys_move_cursor(1, print_location);
    printf("> [TASK] I want to acquire a mute lock from task(pid=%ld).", (long)other_pid);

    mthread_mutex_lock(&lock1);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I have acquired a mutex lock from task(pid=%ld).", (long)other_pid);

    mthread_mutex_unlock(&lock1);

    sys_exit(); // test exit
}

void wait_exit_task()
{
    struct task_info task1 = {(uintptr_t)&ready_to_exit_task, USER_PROCESS};
    struct task_info task2 = {(uintptr_t)&wait_lock_task, USER_PROCESS};
    mthread_mutex_init(&lock1);
    mthread_mutex_init(&lock2);

    pid_t pid_task1 = sys_spawn(&task1, NULL, ENTER_ZOMBIE_ON_EXIT);
    pid_t pid_task2 = sys_spawn(&task2, (void*)(long)pid_task1,
                                AUTO_CLEANUP_ON_EXIT);

    int print_location = 3;

    sys_move_cursor(1, print_location);
    printf("> [TASK] I want to wait task (pid=%ld) to exit.",
           pid_task1);

    sys_waitpid(pid_task1); //test waitpid

    sys_move_cursor(1, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ",
           pid_task1);

    sys_exit(); // test exit
}
