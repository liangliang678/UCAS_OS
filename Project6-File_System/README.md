# Project5: Device Driver

## TODO
1. 硬链接和软链接。
2. 跨block读写的测试。
3. 大文件的支持。
4. 代码的优化和检查。

## 实验说明
### 开发环境
本实验用到的RISC-V开发板是XILINX PYNQ Z2开发板，开发板上有ARM核和FPGA，RISC-V核烧写到FPGA中。
在开发板上电时由ARM核启动相关程序(BOOT.bin)，将RISC-V核烧入FPGA。
之后，RISC-V核自动加载相关程序。RISC-V核为采用SiFive开源的Rocket核心。其基本信息为：
1. RV64IMAFDC(GC)指令集
2. 双核五级流水处理器
3. 数据/指令缓存大小各为16KB，4路组相联
4. 时钟频率为50MHz

另外我们需要一张SD卡。将SD卡格式化为两个分区：第一个分区34MB，采用fat32文件系统；
剩余空间全部划入第二个分区，保持第二个分区为空分区。
制作完以后，将预先提供的BOOT.bin和RV_BOOT.bin拷入到第一个分区。

### 运行方法
拿到裸开发板以后，需要将开发板按如下方式配置：
1. 将开发板设置为SD卡启动。PYNQ Z2支持从SD、QSPI和JTAG启动。我们将跳线插到最左侧，选择用SD卡启动。
2. 设置电源选项。PYNQ Z2支持电源供电和Micro-USB供电。我们把跳线插到USB这个选项上，用Micro-USB直接供电。

配置完开发板后：
1. 将SD卡插入电脑中，将Makefile中的DISK的路径改为SD卡所在的路径。
2. 执行make all命令和make floppy命令，将内核镜像拷贝到SD卡中。
3. 将SD卡插入开发板中，使用Micro-USB线连接电脑和开发板。该线同时兼顾供电和传输数据的功能。
4. 打开开发板电源开关，开发板上会有红色和绿色的指示灯亮起，表明开发板已开始工作。
5. 使用minicom连接开发板，然后按下开发板的RESET按键。
6. 在minicom中出现bbl字样后，输入loadbootm以双核方式启动操作系统。

---
## 内核部分说明
### 相关目录结构
| 文件夹 | 说明 |
| :--- | :--- |
| ./arch/riscv | 体系结构相关代码 |
| ./drivers | 驱动代码 |
| ./include/os | 头文件 |
| ./init | 初始化代码 |
| ./kernel | 内核主要文件 |
| ./libs | 内核使用的库 |

### 初始化

### 进程调度

### 中断处理

### 同步与通信

### 内存管理

### 屏幕驱动

### 网卡驱动
相关文件：./drivers/net.c，./drivers/net.h，./drivers/plic.c，./drivers/plic.h，./drivers/emacps

---
## 用户部分说明
### shell支持的命令
| 命令 | 参数 | 说明 |
| :---: | :---: | :--- |
| ps |  无 | 显示所有进程的pid，状态，MASK和运行在哪个核上 |
| ls | 无 | 显示所有可以执行的任务的名称 |
| reset | 无 | 清空下半屏（命令行） |
| clear | 无 | 清空上半屏（用户输出） |
| exec | name | 将名称为name的任务启动 |
| kill | pid | 杀死进程号为pid的进程 |
| taskset | -p mask pid | 设置进程号为pid的进程的MASK |

注：MASK=1表示进程被允许运行在core 0；MASK=2表示进程被允许运行在core 1；MASK=3表示进程被允许运行在core 0和core 1。

### 编写用户程序
1. 将编写的.c文件置于./test目录下
2. 在Makefile第41行的SRC_USER中添加相应的.elf文件
3. 重新编译后，在shell中使用ls命令可以输出新增的文件名称，说明添加成功

### 库函数一览表
| 函数 | 说明 | 头文件 |
| :--- |  :--- | :---: |
| clock_t clock() | 返回当前CPU运行的时钟周期数 | time.h |
| int printf(const char *fmt, ...) | 用户态打印函数 | stdio.h |
| void srand(unsigned seed) | 设置随机数种子 | stdlib.h |
| int rand() | 返回一个随机数 | stdlib.h |
| void* memcpy(void *dest, const void *src, size_t len) | 将地址src的前len个字节拷贝到地址dest | string.h |
| void* memset(void *dest, int val, size_t len) | 将地址dest后的len个字节的值设置为val | string.h |
| int memcmp(const void *ptr1, const void *ptr2, size_t num) | 比较地址ptr1和ptr2后的num个字符 | string.h |
| size_t strlen(const char *src) | 返回字符串src的长度 | string.h |
| int strcmp(const char *str1, const char *str2) | 比较字符串str1和str2 | string.h |
| char *strcpy(char *dest, const char *src) | 将字符串src拷贝到字符串dest | string.h |
| char *strcat(char *dest, const char *src) | 将字符串dest拼接到字符串src之后 | string.h |
| int atoi(char* src) | 将字符串src转换为整型数字 | string.h |
| long int atol(const char* str) | 将字符串src转换为长整型数字 | stdlib.h |
| int mbox_open(char *name) | 打开指定名字的信箱，返回该信箱的ID | mailbox.h |
| void mbox_close(int mailbox_id) | 关闭信箱 | mailbox.h |
| void mbox_send(int mailbox_id, void *msg, int msg_length) | 向信箱中发送数据 | mailbox.h |
| void mbox_recv(int mailbox_id, void *msg, int msg_length) | 从信箱中接收数据 | mailbox.h |
| void mthread_barrier_init(mthread_barrier_t * barrier, unsigned count) | 初始化屏障 | mthread.h |
| void mthread_barrier_wait(mthread_barrier_t *barrier) | 使用屏障进行同步操作 | mthread.h |
| void mthread_barrier_destroy(mthread_barrier_t *barrier) | 关闭屏障 | mthread.h |
| int mthread_cond_init(mthread_cond_t *cond) | 初始化条件变量 | mthread.h |
| int mthread_cond_destroy(mthread_cond_t *cond) | 关闭条件变量 | mthread.h |
| int mthread_cond_wait(mthread_cond_t *cond, int binsem_id) | 等待条件变量 | mthread.h |
| int mthread_cond_signal(mthread_cond_t *cond) | 唤醒一个等待条件变量的进程 | mthread.h |
| int mthread_cond_broadcast(mthread_cond_t *cond)  | 唤醒全部等待条件变量的进程 | mthread.h |

注：其他未被列在表格中的头文件：alloca.h, assert.h, os.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, sys/syscall.h

### 系统调用一览表

---
## 其他说明