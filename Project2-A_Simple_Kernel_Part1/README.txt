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
内核代码，本部分实验涉及任务切换等内容
| | |--head.S
为操作系统准备C语言环境
| | |--trap.S
本部分实验暂不使用
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
目录，中断处理相关，本部分实验暂不使用
| |--locking
目录，锁相关
| | |--futex.c
| | |--lock.c
| |--mm
目录，内存管理相关
| |--sched
目录，进程调度相关
| | |--sched.c
任务的调度相关，一个任务的调度、挂起、唤醒等逻辑
| | |--time.c
时间相关函数，本部分实验暂不使用
| |--syscall
目录，系统调用，本部分实验暂不使用
|--libs
目录，提供的库函数
|--test
目录，实验的测试任务
|--tiny_libc
目录，本部分实验暂不使用
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

主要文件的功能：
一、main.c
(1) 初始化PCB和内核栈
(2) 初始化屏幕输出
二、sched.c
(1) 实现进程的切换
(2) 实现进程的阻塞
(3) 实现进程的解除阻塞
三、entry.S
(1) 启用/禁用抢占
(2) 启用/禁用中断
(3) switch_to函数的实现：保存和恢复现场
四、lock.c
(1) 自旋锁的初始化、获取、释放
(2) 互斥锁的初始化、获取、释放