#include <time.h>
#include <stdio.h>
#include <test3.h>
#include <sys/syscall.h>
#include <mthread.h>
#include <stdlib.h>

#define MAX_ITERATION 100
#define INTEGER_TEST_CHUNK 100000
#define INTEGER_TEST_NUM 5

void integer_test_task(int print_location);

void test_affinity(void)
{
    srand(42);
    sys_move_cursor(1, 1);
    printf("start test cpu affinity, pids = {");
    int single_core_result = 0;
    struct task_info task_test = {(uintptr_t)&integer_test_task, USER_PROCESS};
    pid_t pids[INTEGER_TEST_NUM] = {0};
    for (int i = 0; i < INTEGER_TEST_NUM; ++i) {
        pids[i] = sys_spawn(&task_test, 2 + i, ENTER_ZOMBIE_ON_EXIT);
        printf("%d, ", pids[i]);
    }
    printf("}\n\r");
    for (int i = 0; i < INTEGER_TEST_NUM; ++i) {
        sys_waitpid(pids[i]);
    }

    sys_exit();
}

void integer_test_task(int print_location)
{
    pid_t pid = sys_getpid();
    uint64_t ans = 0;
    for (int i = 0; i < MAX_ITERATION; ++i) {
        for (int j = 0; j < INTEGER_TEST_CHUNK; ++j) {
            ans += rand();
	    }
        sys_move_cursor(1, print_location);
        printf("[%ld] integer test (%d/%d)\n\r", pid, i, MAX_ITERATION);
    }
    sys_exit();
}
