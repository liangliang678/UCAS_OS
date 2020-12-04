#include <sys/syscall.h>
#include <stdint.h>

pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_EXEC, (uintptr_t)file_name, argc, (uintptr_t)argv, mode);
}

void sys_exit(void)
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE, IGNORE);
}

int sys_kill(pid_t pid)
{
    return invoke_syscall(SYSCALL_KILL, pid, IGNORE, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
    return invoke_syscall(SYSCALL_WAITPID, pid, IGNORE, IGNORE, IGNORE);
}

void sys_process_show(char* buffer)
{
    invoke_syscall(SYSCALL_PS, (uintptr_t)buffer, IGNORE, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
    return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_yield()
{
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_taskset(pid_t pid, unsigned long mask)
{
    return invoke_syscall(SYSCALL_TASKSET, (uintptr_t)pid, (uintptr_t)mask, IGNORE, IGNORE);
}

void sys_show_exec(char* buffer)
{
    invoke_syscall(SYSCALL_SHOW_EXEC, (uintptr_t)buffer, IGNORE, IGNORE, IGNORE);
}

void sys_futex_wait(volatile uint64_t *val_addr, int binsem_id)
{
    invoke_syscall(SYSCALL_FUTEX_WAIT, (uintptr_t)val_addr, (uintptr_t)binsem_id, IGNORE, IGNORE);
}

void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup)
{
    invoke_syscall(SYSCALL_FUTEX_WAKEUP, (uintptr_t)val_addr, num_wakeup, IGNORE, IGNORE);
}

int sys_binsem_get(int key)
{
    return invoke_syscall(SYSCALL_BINSEM_GET, (uintptr_t)key, IGNORE, IGNORE, IGNORE);
}

void sys_binsem_op(int binsem_id, int op)
{
    invoke_syscall(SYSCALL_BINSEM_OP, (uintptr_t)binsem_id, (uintptr_t)op, IGNORE, IGNORE);
}

void* sys_shmpage_get(int key)
{
    return invoke_syscall(SYSCALL_SHMPGET, key, IGNORE, IGNORE, IGNORE);
}

void sys_shmpage_dt(void *addr)
{
    invoke_syscall(SYSCALL_SHMPDT, (uintptr_t)addr, IGNORE, IGNORE, IGNORE);
}


void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, (uintptr_t)line1, (uintptr_t)line2, IGNORE, IGNORE);
}

void sys_screen_scroll(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_SCROLL, (uintptr_t)line1, (uintptr_t)line2, IGNORE, IGNORE);
}

long sys_get_timebase()
{
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_get_char()
{
    int ch = -1;
    while (ch == -1) {
        ch = invoke_syscall(SYSCALL_GET_CHAR, IGNORE, IGNORE, IGNORE, IGNORE);
    }
    return ch;
}

int sys_mbox_open(char *name)
{
    return invoke_syscall(SYSCALL_MAILBOX_OPEN, (uintptr_t)name, IGNORE, IGNORE, IGNORE);
}

void sys_mbox_close(int mailbox)
{
    invoke_syscall(SYSCALL_MAILBOX_CLOSE, (uintptr_t)mailbox, IGNORE, IGNORE, IGNORE);
}

int sys_mbox_send(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_SEND, (uintptr_t)mailbox, (uintptr_t)msg, (uintptr_t)msg_length, IGNORE);
}

int sys_mbox_recv(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_RECV, (uintptr_t)mailbox, (uintptr_t)msg, (uintptr_t)msg_length, IGNORE);
}