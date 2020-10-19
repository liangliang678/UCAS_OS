#include <stdio.h>
#include <sys/syscall.h>

void priority_task1(void)
{
    int i;
    int print_location = 1;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task2(void)
{
    int i;
    int print_location = 2;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task3(void)
{
    int i;
    int print_location = 3;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task4(void)
{
    int i;
    int print_location = 4;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task5(void)
{
    int i;
    int print_location = 5;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task6(void)
{
    int i;
    int print_location = 6;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task7(void)
{
    int i;
    int print_location = 7;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task8(void)
{
    int i;
    int print_location = 8;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task9(void)
{
    int i;
    int print_location = 9;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}

void priority_task10(void)
{
    int i;
    int print_location = 10;

    for (i = 0;; i++)
    {
        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is to test priority. (%d)", i);
    }
}