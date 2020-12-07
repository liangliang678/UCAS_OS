编写用户程序的步骤：
1. 将编写的.c文件置于./test目录下
2. 在Makefile第39行的SRC_USER中添加相应的.elf文件
3. 重新编译运行后，使用ls命令可以输出新增的文件名称，说明添加成功

shell支持的命令：
-------------------------------------------------------------------
命令         参数                       说明
-------------------------------------------------------------------
ps           无            显示所有进程的pid，状态，MASK和运行在哪个核上
ls           无            显示所有可以执行的任务的名称
reset        无            清空下半屏（命令行）
clear        无            清空上半屏（用户输出）
exec         name          将名称为name的任务启动
kill         pid           杀死进程号为pid的进程
taskset      -p mask pid   设置进程号为pid的进程的MASK
-------------------------------------------------------------------
注：MASK=1表示进程被允许运行在core 0；MASK=2表示进程被允许运行在core 1；MASK=3表示进程被允许运行在core 0和core 1；

提供的库函数：
--------------------------------------------------------------------------------------------------------------------------------------
函数                                                                                 说明                          头文件
--------------------------------------------------------------------------------------------------------------------------------------
clock_t clock()                                                          返回当前CPU运行的时钟周期数             time.h
int printf(const char *fmt, ...)                                         用户态打印函数                         stdio.h
void srand(unsigned seed)                                                设置随机数种子                         stdlib.h
int rand()                                                               返回一个随机数                         stdlib.h

void* memcpy(void *dest, const void *src, size_t len)                    将地址src的前len个字节拷贝到地址dest    string.h
void* memset(void *dest, int val, size_t len)                            将地址dest后的len个字节的值设置为val    string.h
int memcmp(const void *ptr1, const void *ptr2, size_t num)               比较地址ptr1和ptr2后的num个字符         string.h
size_t strlen(const char *src)                                           返回字符串src的长度                    string.h
int strcmp(const char *str1, const char *str2)                           比较字符串str1和str2                   string.h
char *strcpy(char *dest, const char *src)                                将字符串src拷贝到字符串dest             string.h
char *strcat(char *dest, const char *src)                                将字符串dest拼接到字符串src之后         string.h
int atoi(char* src)                                                      将字符串src转换为整型数字               string.h
long int atol(const char* str)                                           将字符串src转换为长整型数字             stdlib.h

int mbox_open(char *name)                                                打开指定名字的信箱，返回该信箱的ID       mailbox.h
void mbox_close(int mailbox_id)                                          关闭信箱                               mailbox.h
void mbox_send(int mailbox_id, void *msg, int msg_length)                向信箱中发送数据                        mailbox.h
void mbox_recv(int mailbox_id, void *msg, int msg_length)                从信箱中接收数据                        mailbox.h

void mthread_barrier_init(mthread_barrier_t * barrier, unsigned count)   初始化屏障                             mthread.h
void mthread_barrier_wait(mthread_barrier_t *barrier)                    使用屏障进行同步操作                    mthread.h
void mthread_barrier_destroy(mthread_barrier_t *barrier)                 关闭屏障                               mthread.h

int mthread_cond_init(mthread_cond_t *cond)                              初始化条件变量                         mthread.h
int mthread_cond_destroy(mthread_cond_t *cond)                           关闭条件变量                           mthread.h
int mthread_cond_wait(mthread_cond_t *cond, int binsem_id)               等待条件变量                           mthread.h
int mthread_cond_signal(mthread_cond_t *cond)                            唤醒一个等待条件变量的进程              mthread.h
int mthread_cond_broadcast(mthread_cond_t *cond)                         唤醒全部等待条件变量的进程              mthread.h
--------------------------------------------------------------------------------------------------------------------------------------
其他未被列在表格中的头文件：alloca.h, assert.h, os.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, sys/syscall.h