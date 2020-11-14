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
| | |--smp.S
双核操作中需要用到的汇编代码
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
内核专用自旋锁
| |--mm
目录，内存管理相关
| |--sched
目录，进程调度相关
| | |--sched.c
任务的调度相关，一个任务的调度、挂起、唤醒等逻辑
| | |--time.c
时间相关函数
| | |--smp.c
从核的启动和初始化相关
| |--syscall
目录，系统调用
| | |--syscall.c
系统调用处理函数
| | |--mailbox.c
进程间通信实现
|--libs
目录，提供的库函数
|--test
目录，用户代码
| |--test_project3
目录，实验的测试任务
| |--test_shell.c
终端的代码
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

shell支持的命令列表：
-------------------------------------------------------------------
命令         参数                       说明
-------------------------------------------------------------------
ps           无        显示所有进程的pid，状态，MASK 和运行在哪个核上
reset        无                  清空下半屏（命令行）
clear        无                 清空上半屏（用户输出）
exec       tasknum             将第tasknum 个任务启动
kill         pid               杀死进程号为pid 的进程
taskset  mask tasknum      将第tasknum 个任务启动并设置MASK
         -p mask pid         设置进程号为pid 的进程的MASK
-------------------------------------------------------------------