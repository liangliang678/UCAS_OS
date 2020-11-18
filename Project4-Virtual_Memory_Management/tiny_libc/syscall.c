#include <sys/syscall.h>
#include <sys/shm.h>
#include <stdint.h>

pid_t sys_spawn(uintptr_t info, void* arg, spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_SPAWN, (uintptr_t)info,
                          (uintptr_t) arg, mode, IGNORE);
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

pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_EXEC, (uintptr_t)file_name, argc, (uintptr_t)argv, mode);
}

void sys_show_exec()
{
    invoke_syscall(SYSCALL_SHOW_EXEC, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (uintptr_t)buff, IGNORE, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE, IGNORE);
}

void sys_futex_wait(volatile uint64_t *val_addr, uint64_t val)
{
    invoke_syscall(SYSCALL_FUTEX_WAIT, (uintptr_t)val_addr, val, IGNORE, IGNORE);
}

void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup)
{
    invoke_syscall(SYSCALL_FUTEX_WAKEUP, (uintptr_t)val_addr, num_wakeup, IGNORE, IGNORE);
}

long sys_get_timebase()
{
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick()
{
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_process_show(void)
{
    invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, line1, line2, IGNORE, IGNORE);
}

pid_t sys_getpid()
{
    return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_get_char()
{
    int ch = -1;
    while (ch == -1) {
        ch = invoke_syscall(SYSCALL_GET_CHAR, IGNORE, IGNORE, IGNORE, IGNORE);
    }
    return ch;
}

void* shmpageget(int key)
{
    return invoke_syscall(SYSCALL_SHMPGET, key, IGNORE, IGNORE, IGNORE);
}

void shmpagedt(void *addr)
{
    invoke_syscall(SYSCALL_SHMPDT, (uintptr_t)addr, IGNORE, IGNORE, IGNORE);
}
