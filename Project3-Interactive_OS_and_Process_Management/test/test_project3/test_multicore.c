#include <time.h>
#include <stdio.h>
#include <test3.h>
#include <sys/syscall.h>
#include <mthread.h>
#include <stdlib.h>

#define MAX_RANGE 5000000
#define MOD 1000007
#define NUM_CPUS 2

struct TestMultiCoreArg
{
    int print_location;
    int from;
    int to;
    int* result;
};
void add_task(struct TestMultiCoreArg* args);

void test_multicore(void)
{
    sys_move_cursor(1, 1);
    printf("start test multi-core performance\n\r");
    int single_core_result = 0;
    struct task_info task_add = {(uintptr_t)&add_task, USER_PROCESS};
    struct TestMultiCoreArg singleCoreArg = {1, 0, MAX_RANGE, &single_core_result};
    // single core performance
    clock_t singleCoreBegin = clock();
    pid_t single_pid = sys_spawn(&task_add, (void*)&singleCoreArg, ENTER_ZOMBIE_ON_EXIT);
    sys_waitpid(single_pid);
    clock_t singleCoreEnd = clock();
    sys_move_cursor(1, 6);
    printf("single core: %ld ticks, result = %d            \n\r", singleCoreEnd - singleCoreBegin, single_core_result);

    struct TestMultiCoreArg multiCoreArgs[NUM_CPUS];
    pid_t pids[NUM_CPUS];
    int multi_core_results[NUM_CPUS] = {0};
    for (int i = 0; i < NUM_CPUS; ++i) {
        multiCoreArgs[i].print_location = i + 2;
        multiCoreArgs[i].from = MAX_RANGE * i / NUM_CPUS;
        multiCoreArgs[i].to = MAX_RANGE * (i + 1) / NUM_CPUS;
        multiCoreArgs[i].result = &multi_core_results[i];
    }

    clock_t multiCoreBegin = clock();
    for (int i = 0; i < NUM_CPUS; ++i) {
        pids[i] = sys_spawn(&task_add, (void*)&multiCoreArgs[i], ENTER_ZOMBIE_ON_EXIT);
    }

    for (int i = 0; i < NUM_CPUS; ++i) {
        sys_waitpid(pids[i]);
    }
    int multi_core_final_result = 0;
    for (int i = 0; i < NUM_CPUS; ++i) {
        multi_core_final_result += multi_core_results[i];
	    multi_core_final_result = multi_core_final_result % MOD;
    }
    clock_t multiCoreEnd = clock();
    sys_move_cursor(1, 7);
    printf("multi core: %ld ticks, result = %d             \n\r", multiCoreEnd - multiCoreBegin, multi_core_final_result);

    sys_exit();
}

void add_task(struct TestMultiCoreArg* args)
{
    int print_location = args->print_location;
    int from  = args->from;
    int to = args->to;
    int result = 0;

    sys_move_cursor(1, print_location);
    printf("start compute, from = %d, to = %d  ", from, to);
    for (int i = from; i < to; ++i) {
        result = (result + i) % MOD;
    }
    printf("Done \n\r");
    *args->result = result;
    sys_exit();
}
