Remaining Questions:
1. （已解决）我们使用PCB中的preempt_count记录当前进程调用disable_preempt()的次数，但是有时disable_preempt()和enable_preempt()中间进行了do_shceduler()（比如futex_wait()），导致前后修改了不同PCB的preempt_count。
2. C-core 要求统计一次do_scheduler函数完成的时间，指的是单独的do_scheduler函数的时间（包括修改tp，修改current_running，优先级调度等），还是整个中断的时间（包括保存寄存器，刷屏，do_scheduler等）？
3. mthread.h中的mthread_mutex_t类型数据（即mthread.c中传入函数的lock）是程序使用的变量，还是我们维护的锁的状态？如果是前者，锁的状态存在哪里？
4. 板子的频率是1MHz，实测时间片为15000比较合理，1秒200个时间片（5000）是不是太多了？
5. 内核进程/线程不应直接调用do_block，do_unblock，do_scheduler，内核态的互斥锁难以实现。