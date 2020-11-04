bugs:
1. 进程退出时未回收内存空间

新增的文件：
/arch/riscv/kernel/smp.S
/include/os/smp.h
/include/os/stdio.h
/kernel/sched/smp.c
/test/test_shell.c
/test/test_project3/*
/tiny_lib/mailbox.c
/tiny_lib/rand.c
/tiny_lib/include/mailbox.h
/tiny_lib/include/os.h
/tiny_lib/include/stddef.h
/tiny_lib/include/stdlib.h

已合并更改的文件：
csr.h
assert.h
mm.h
sched.h
syscall_number.h
syscall.h
irq.c
printk.c
string.c
test.c
test.h
mthread.c
string.c
mthread.h
stdatomic.h
stdio.h
string.h

未合并更改的文件：
bootblock.S