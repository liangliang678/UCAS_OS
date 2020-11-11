#ifndef INCLUDE_TASK3_H_
#define INCLUDE_TASK3_H_

// [TASK2]
void ready_to_exit_task(void);
void wait_lock_task();
void wait_exit_task();

// [TASK3]
void test_condition(void);
void producer_task(void);
void consumer_task(int print_location);

void semaphore_add_task1(void);
void semaphore_add_task2(void);
void semaphore_add_task3(void);

void test_barrier(void);
void barrier_task(int print_location);

// [TASK4]
void SunQuan(void);
void LiuBei(void);
void CaoCao(void);

// [MULTI-CORE]
void test_multicore(void);
void test_affinity(void);

#endif
