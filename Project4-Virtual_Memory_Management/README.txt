�Ѻϲ��ĸ��ģ�
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

δ�ϲ��ĸ��ģ�
include/os/mm.h
kernel/mm/mm.c
kernel/syscall/syscall.c
tiny_libc/include/string.h
tiny_libc/string.c

�������ļ���
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

��Ҫ�޸ĵ��ļ���
Makefile

�ο����ϣ�https://osblog.stephenmarz.com/ch3.2.html

Questions:
1. Aλ��Dλ�����ã�
2. �����ں˺�Ҫ��Ҫ�л�ҳ������Ҫ��

TODO��
1. �޸�shell�е�exec��do_exec
2. �޸�waitpid��ϵͳ�����Լ����kmalloc
3. �����ڴ�