#include "test.h"

/* [TASK1] task group to test do_scheduler() */
struct task_info task2_1 = {(ptr_t)&printk_task1, KERNEL_THREAD, 0};
struct task_info task2_2 = {(ptr_t)&printk_task2, KERNEL_THREAD, 0};
struct task_info task2_3 = {(ptr_t)&drawing_task1, KERNEL_THREAD, 0};
struct task_info *sched1_tasks[16] = {&task2_1, &task2_2, &task2_3};
int num_sched1_tasks = 3;

/* [TASK2] task group to test lock */
// test_lock1.c : Kernel space lock test
// test_lock2.c, test_lock2_binsem.c : User space lock test
struct task_info task2_4 = {(ptr_t)&lock_task1, KERNEL_THREAD, 0};
struct task_info task2_5 = {(ptr_t)&lock_task2, KERNEL_THREAD, 0};
struct task_info *lock_tasks[16] = {&task2_4, &task2_5};
int num_lock_tasks = 2;

/* [TASK3] [TASK4] task group to test interrupt */
struct task_info task2_6 = {(ptr_t)&sleep_task, USER_PROCESS, 0};
struct task_info task2_7 = {(ptr_t)&timer_task, USER_PROCESS, 0};
struct task_info *timer_tasks[16] = {&task2_6, &task2_7};
int num_timer_tasks = 2;

struct task_info task2_8 = {(ptr_t)&printf_task1, USER_PROCESS, 0};
struct task_info task2_9 = {(ptr_t)&printf_task2, USER_PROCESS, 0};
struct task_info task2_10 = {(ptr_t)&drawing_task2, USER_PROCESS, 0};
struct task_info task2_11 = {(ptr_t)&deadlock_task, USER_PROCESS, 0};
struct task_info *sched2_tasks[16] = {&task2_8, &task2_9, &task2_10, &task2_11};
int num_sched2_tasks = 4;

struct task_info task2_12 = {(ptr_t)&lock2_task1, USER_THREAD, 0};
struct task_info task2_13 = {(ptr_t)&lock2_task2, USER_THREAD, 0};
struct task_info task2_14 = {(ptr_t)&lock2_task3, USER_PROCESS, 0};
struct task_info task2_15 = {(ptr_t)&lock2_task4, USER_PROCESS, 0};
struct task_info *lock2_tasks[16] = {&task2_12, &task2_13, &task2_14, &task2_15};
int num_lock2_tasks = 4;

/* [TASK5] task group to test priority */
struct task_info task2_16 = {(ptr_t)&priority_task1, USER_PROCESS, 104};
struct task_info task2_17 = {(ptr_t)&priority_task2, USER_PROCESS, 90};
struct task_info task2_18 = {(ptr_t)&priority_task3, USER_PROCESS, 82};
struct task_info task2_19 = {(ptr_t)&priority_task4, USER_PROCESS, 73};
struct task_info task2_20 = {(ptr_t)&priority_task5, USER_PROCESS, 69};
struct task_info task2_21 = {(ptr_t)&priority_task6, USER_PROCESS, 56};
struct task_info task2_22 = {(ptr_t)&priority_task7, USER_PROCESS, 41};
struct task_info task2_23 = {(ptr_t)&priority_task8, USER_PROCESS, 34};
struct task_info task2_24 = {(ptr_t)&priority_task9, USER_PROCESS, 23};
struct task_info task2_25 = {(ptr_t)&priority_task10, USER_PROCESS, 10};
struct task_info *priority_tasks[16] = {&task2_16, &task2_17, &task2_18, &task2_19, &task2_20, 
                                        &task2_21, &task2_22, &task2_23, &task2_24, &task2_25};
int num_priority_tasks = 10;