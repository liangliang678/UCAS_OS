#include <time.h>
#include <test3.h>
#include <stdio.h>
#include <os.h>
#include <stddef.h>
#include <sys/syscall.h>

#define LOCK1_BINSEM_KEY 42
#define LOCK2_BINSEM_KEY 24

void ready_to_exit_task()
{
    int i = 0, print_location = 1;

    int lock1_id = sys_binsem_get(LOCK1_BINSEM_KEY);
    int lock2_id = sys_binsem_get(LOCK2_BINSEM_KEY);

    sys_binsem_op(lock1_id, BINSEM_OP_LOCK);
    sys_binsem_op(lock2_id, BINSEM_OP_LOCK);

    for (i = 0; i < 1000; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] I am task with pid %d, I have acquired two binsem lock. (%d)", sys_getpid(), i++);
    }

    sys_exit(); // test exit
}

void wait_lock_task(long other_pid)
{
    int print_location = 2;

    sys_move_cursor(1, print_location);
    printf("> [TASK] I want to acquire a binsem lock from task(pid=%ld).", (long)other_pid);

    int lock1_id = sys_binsem_get(LOCK1_BINSEM_KEY);
    sys_binsem_op(lock1_id, BINSEM_OP_LOCK);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I have acquired a mutex lock from task(pid=%ld).   ", (long)other_pid);

    sys_binsem_op(lock1_id, BINSEM_OP_UNLOCK);

    sys_exit(); // test exit
}

void wait_exit_task()
{
    struct task_info task1 = {(uintptr_t)&ready_to_exit_task, USER_PROCESS};
    struct task_info task2 = {(uintptr_t)&wait_lock_task, USER_PROCESS};

    pid_t pid_task1 = sys_spawn(&task1, NULL, ENTER_ZOMBIE_ON_EXIT);
    sys_spawn(&task2, (void*)(long)pid_task1, AUTO_CLEANUP_ON_EXIT);

    int print_location = 3;

    sys_move_cursor(1, print_location);
    printf("> [TASK] I want to wait task (pid=%ld) to exit.", pid_task1);

    sys_waitpid(pid_task1); //test waitpid

    sys_move_cursor(1, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid_task1);

    sys_exit(); // test exit
}
