本项目的目录结构如下，部分不重要的文件/文件夹未列出：

|--arch
目录，RISC-V架构相关内容，主要为汇编代码以及相关宏定义
| |--boot
| | |--bootblock.S
引导代码
| |--include
目录，头文件，包括一些宏定义
| |--kernel
目录，内核中需要汇编实现的部分
| | |--entry.S
内核代码，包括开关中断，进入和退出中断等内容
| | |--head.S
为操作系统准备C语言环境
| | |--trap.S
设置中断
| |--sbi
| | |--common.c
SBI调用以及功能寄存器的访问
|--drivers
目录，驱动相关代码
|--include
目录，头文件
|--init
目录，初始化相关
| |--main.c
内核的入口，操作系统的起点
|--kernel
目录，内核相关文件
| |--irq
目录，中断处理相关
| | |--irq.c
分发例外处理函数，处理时钟中断
| |--locking
目录，锁相关
| | |--binsem.c
二元信号量实现互斥锁
| | |--futex.c
Futex机制实现互斥锁
| | |--lock.c
内核态专用锁
| |--mm
目录，内存管理相关
| |--sched
目录，进程调度相关
| | |--sched.c
任务的调度相关，一个任务的调度、挂起、唤醒等逻辑
| | |--time.c
时间相关函数
| |--syscall
目录，系统调用
| | |--syscall.c
系统调用处理函数
|--libs
目录，提供的库函数
|--test
目录，实验的测试任务
|--tiny_libc
目录，用户程序允许使用的库函数
|--tools
目录，工具（createimage）
|--bootblock
|--createimage
|--design_document.pdf
设计文档
|--image
|--kernel.txt
|--main
|--Makefile
Makefile文件
|--README.txt
本文件
|--riscv.lds
链接器脚本文件

函数调用约定：
内核进程/线程可以调用的函数：do_scheduler(), do_block(), do_unblock(), printk()等
用户进程/线程可以调用的函数：sys_*(), mthread_spin_*(), mthread_mutex_*(), printf()等

注意事项：
1. 若不需要测量scheduler()函数时间的功能，请注释sched.c中的第40行和第63行以提高运行速度。在内核中使用printk会使时钟中断处理非常缓慢。
2. 测量scheduler()函数时间的功能将占用屏幕的13，14，15行输出，如有需要可以在sched.c第120行处修改打印位置。
3. 在main.c中有三组可以加载的PCB，第一组是Part1的测试用例，第二组是Part2的测试用例，第三组是优先级调度的测试用例。
4. 在main.c中第189行处可以修改一个时间片的tick数，目前1s内约有200个时间片。

更多内容请参考design_document.pdf