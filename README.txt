本项目的目录结构如下：
|--include/
目录，描述SBI提供的函数。
| |--asm/
目录，包含宏SBI_CALL的相关内容
| | |--sbiasm.h
宏SBI_CALL的定义
| | |--sbidef.h
宏SBI_CALL可供调用的接口类型定义
| |--sbi.h
SBI提供函数的C语言版本
|--bootblock
bootblock的ELF文件
|--bootblock.S
引导程序代码
|--createimage
将bootblock和kernel制作成引导块的工具
|--createimage.c
引导块工具代码
|--Design Document.pdf
设计文档
|--disk
供qemu使用的内核镜像
|--elf2char
|--generateMapping
|--head.S
内核入口代码，负责准备C语言执行环境
|--image
内核镜像
|--kernel
kernel的ELF文件
|--kernel.c
内核的代码
|--Makefile
Makefile文件
|--README.txt
本文件
|--riscv.lds
链接器脚本文件

主要文件的功能：
一、bootblock.S
(1) 打印字符串It's a bootloader...”。
(2) 将位于SD第二个扇区的内核代码段移动至内存。
(3) 跳转到内核代码的入口。
二、head.S
(1) 清空BSS段。
(2) 设置栈指针。
(3) 完成bootblock在移动内核代码后尚未完成的工作。
(4) 重定位。
三、kernel.c
(1) 输出字符串“Hello OS”。
(2) 连续接收输入，并同时把输入的字符打印出来。
四、createimage.c
(1) 将bootblock和kernel结合为一个操作系统镜像。
(2) 打印操作系统镜像的一些信息。
(3) 将kernel段的大小写入镜像中特定位置供引导块执行时取用。