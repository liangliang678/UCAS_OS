#include <test2.h>
#include <mthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/syscall.h>

/*
 * LOCK2_TASK_KEY is the key of this task. You can define it as you wish.
 * We use 42 here because it is "Answer to the Ultimate Question of Life,
 * the Universe, and Everything" :)
 */
#define LOCK2_BINSEM_KEY 42

static char blank[] = {"                                             "};

void lock2_task1(void)
{
        int print_location = 3;
	int binsem_id = sys_binsem_get(LOCK2_BINSEM_KEY);
        while (1)
        {
                int i;

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for a lock.\n");

                sys_binsem_op(binsem_id, BINSEM_OP_LOCK);

                for (i = 0; i < 1000; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock and exited.\n");

                sys_binsem_op(binsem_id, BINSEM_OP_UNLOCK);
        }
}

void lock2_task2(void)
{
        int print_location = 4;
	int binsem_id = sys_binsem_get(LOCK2_BINSEM_KEY);
        while (1)
        {
                int i;

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for a lock.\n");

                sys_binsem_op(binsem_id, BINSEM_OP_LOCK);

                for (i = 0; i < 1000; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock and exited.\n");

                sys_binsem_op(binsem_id, BINSEM_OP_UNLOCK);
        }
}
