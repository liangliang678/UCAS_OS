#include <sys/syscall.h>
#include <stdint.h>

pid_t sys_spawn(task_info_t *info, void* arg, spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_SPAWN, (uintptr_t)info, (uintptr_t)arg, (uintptr_t)mode);
}

void sys_exit(void)
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

int sys_kill(pid_t pid)
{
    return invoke_syscall(SYSCALL_KILL, (uintptr_t)pid, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
    return invoke_syscall(SYSCALL_WAITPID, (uintptr_t)pid, IGNORE, IGNORE);
}

void sys_process_show(char* buffer)
{
    return invoke_syscall(SYSCALL_PS, (uintptr_t)buffer, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
    return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}

void sys_yield()
{
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE);
}

int sys_taskset(pid_t pid, unsigned long mask)
{
    return invoke_syscall(SYSCALL_TASKSET, (uintptr_t)pid, (uintptr_t)mask, IGNORE);
}

void sys_futex_wait(volatile uint64_t *val_addr)
{
    invoke_syscall(SYSCALL_FUTEX_WAIT, (uintptr_t)val_addr, IGNORE, IGNORE);
}

void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup)
{
    invoke_syscall(SYSCALL_FUTEX_WAKEUP, (uintptr_t)val_addr, num_wakeup, IGNORE);
}

int sys_binsem_get(int key)
{
    return invoke_syscall(SYSCALL_BINSEM_GET, (uintptr_t)key, IGNORE, IGNORE);
}

void sys_binsem_op(int binsem_id, int op)
{
    invoke_syscall(SYSCALL_BINSEM_OP, (uintptr_t)binsem_id, (uintptr_t)op, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, (uintptr_t)line1, (uintptr_t)line2, IGNORE);
}

void sys_screen_scroll(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_SCROLL, (uintptr_t)line1, (uintptr_t)line2, IGNORE);
}

long sys_get_timebase()
{
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE);
}

int sys_get_char()
{
    return invoke_syscall(SYSCALL_GET_CHAR, IGNORE, IGNORE, IGNORE);
}

int sys_mbox_open(char *name)
{
    return invoke_syscall(SYSCALL_MAILBOX_OPEN, name, IGNORE, IGNORE);
}

void sys_mbox_close(int mailbox)
{
    invoke_syscall(SYSCALL_MAILBOX_CLOSE, mailbox, IGNORE, IGNORE);
}

int sys_mbox_send(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_SEND, mailbox, msg, msg_length);
}

int sys_mbox_recv(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_RECV, mailbox, msg, msg_length);
}