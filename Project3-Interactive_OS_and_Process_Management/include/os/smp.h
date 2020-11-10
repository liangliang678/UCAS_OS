#ifndef SMP_H
#define SMP_H

#define NR_CPUS 2

// extern void* cpu_stack_pointer[NR_CPUS];
// extern void* cpu_pcb_pointer[NR_CPUS];
extern void smp_init();
extern void wakeup_other_hart();
extern uint64_t get_current_cpu_id();
extern void lock_kernel();
extern void unlock_kernel();

extern struct spin_lock kernel_lock;
extern volatile int init_flag;

#endif /* SMP_H */
