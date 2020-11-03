#include <time.h>
#include <test3.h>
#include <stdio.h>
#include <sys/syscall.h>

void waiting_task1(void)
{
    int print_location = 1;

    sys_move_cursor(1, print_location);
    printf("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    sys_waitpid(3);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I am waiting task and already exited successfully.\n");

    sys_exit();
}

void waiting_task2(void)
{
    int print_location = 2;

    sys_move_cursor(1, print_location);
    printf("> [TASK] This is a waiting task (pid=%d).\n", print_location);

    sys_waitpid(3);

    sys_move_cursor(1, print_location);
    printf("> [TASK] I am waiting task and already exited successfully.\n");

    sys_exit();
}

void waited_task(void)
{
    int print_location = 3;

    sys_move_cursor(1, print_location);
    printf("> [TASK] I still have 5 seconds to quit.\n");

    sys_sleep(5);

    sys_exit();
}
