��д�û�����Ĳ��裺
1. ����д��.c�ļ�����./testĿ¼��
2. ��Makefile��39�е�SRC_USER�������Ӧ��.elf�ļ�
3. ���±������к�ʹ��ls�����������������ļ����ƣ�˵����ӳɹ�

shell֧�ֵ����
-------------------------------------------------------------------
����         ����                       ˵��
-------------------------------------------------------------------
ps           ��            ��ʾ���н��̵�pid��״̬��MASK���������ĸ�����
ls           ��            ��ʾ���п���ִ�е����������
reset        ��            ����°����������У�
clear        ��            ����ϰ������û������
exec         name          ������Ϊname����������
kill         pid           ɱ�����̺�Ϊpid�Ľ���
taskset      -p mask pid   ���ý��̺�Ϊpid�Ľ��̵�MASK
-------------------------------------------------------------------
ע��MASK=1��ʾ���̱�����������core 0��MASK=2��ʾ���̱�����������core 1��MASK=3��ʾ���̱�����������core 0��core 1��

�ṩ�Ŀ⺯����
--------------------------------------------------------------------------------------------------------------------------------------
����                                                                                 ˵��                          ͷ�ļ�
--------------------------------------------------------------------------------------------------------------------------------------
clock_t clock()                                                          ���ص�ǰCPU���е�ʱ��������             time.h
int printf(const char *fmt, ...)                                         �û�̬��ӡ����                         stdio.h
void srand(unsigned seed)                                                �������������                         stdlib.h
int rand()                                                               ����һ�������                         stdlib.h

void* memcpy(void *dest, const void *src, size_t len)                    ����ַsrc��ǰlen���ֽڿ�������ַdest    string.h
void* memset(void *dest, int val, size_t len)                            ����ַdest���len���ֽڵ�ֵ����Ϊval    string.h
int memcmp(const void *ptr1, const void *ptr2, size_t num)               �Ƚϵ�ַptr1��ptr2���num���ַ�         string.h
size_t strlen(const char *src)                                           �����ַ���src�ĳ���                    string.h
int strcmp(const char *str1, const char *str2)                           �Ƚ��ַ���str1��str2                   string.h
char *strcpy(char *dest, const char *src)                                ���ַ���src�������ַ���dest             string.h
char *strcat(char *dest, const char *src)                                ���ַ���destƴ�ӵ��ַ���src֮��         string.h
int atoi(char* src)                                                      ���ַ���srcת��Ϊ��������               string.h
long int atol(const char* str)                                           ���ַ���srcת��Ϊ����������             stdlib.h

int mbox_open(char *name)                                                ��ָ�����ֵ����䣬���ظ������ID       mailbox.h
void mbox_close(int mailbox_id)                                          �ر�����                               mailbox.h
void mbox_send(int mailbox_id, void *msg, int msg_length)                �������з�������                        mailbox.h
void mbox_recv(int mailbox_id, void *msg, int msg_length)                �������н�������                        mailbox.h

void mthread_barrier_init(mthread_barrier_t * barrier, unsigned count)   ��ʼ������                             mthread.h
void mthread_barrier_wait(mthread_barrier_t *barrier)                    ʹ�����Ͻ���ͬ������                    mthread.h
void mthread_barrier_destroy(mthread_barrier_t *barrier)                 �ر�����                               mthread.h

int mthread_cond_init(mthread_cond_t *cond)                              ��ʼ����������                         mthread.h
int mthread_cond_destroy(mthread_cond_t *cond)                           �ر���������                           mthread.h
int mthread_cond_wait(mthread_cond_t *cond, int binsem_id)               �ȴ���������                           mthread.h
int mthread_cond_signal(mthread_cond_t *cond)                            ����һ���ȴ����������Ľ���              mthread.h
int mthread_cond_broadcast(mthread_cond_t *cond)                         ����ȫ���ȴ����������Ľ���              mthread.h
--------------------------------------------------------------------------------------------------------------------------------------
����δ�����ڱ���е�ͷ�ļ���alloca.h, assert.h, os.h, stdarg.h, stdatomic.h, stdbool.h, stddef.h, stdint.h, sys/syscall.h