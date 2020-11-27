已合并的更改：
arch/riscv/include/atomic.h
arch/riscv/include/sbi.h
arch/riscv/kernel/head.S
include/os/sched.h
include/os/syscall_number.h
libs/string.c
test/test_shell.c
test/test.h
tiny_libc/include/stdio.h
tiny_libc/include/stdlib.h
tiny_libc/printf.c
tiny_libc/syscall.c

未合并的更改：
include/os/mm.h
kernel/mm/mm.c
kernel/syscall/syscall.c
tiny_libc/include/string.h
tiny_libc/string.c

新增的文件：
arch/riscv/include/context.h
arch/riscv/include/pgtable.h
arch/riscv/kernel/boot.c
arch/riscv/kernel/start.S
arch/riscv/crt0.S
include/os/elf.h
include/sys/shm.h
test/consensus.c
test/fly.c
test/rw.c
tiny_libc/include/alloca.h
tiny_libc/atol.c
tiny_libc/invoke_syscall.S
tools/elfchar.c
tools/generateMapping.c
user_riscv.lds

需要修改的文件：
Makefile

参考资料：https://osblog.stephenmarz.com/ch3.2.html

Questions:
1. A位和D位的作用？
2. 进入内核后要不要切换页表（不需要）

TODO：
1. 修改shell中的exec和do_exec
2. 修改waitpid等系统调用以及完成kmalloc
3. 回收内存