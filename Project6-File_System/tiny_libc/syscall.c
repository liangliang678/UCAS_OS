#include <sys/syscall.h>
#include <stdint.h>

pid_t sys_exec(const char *file_name, int argc, char* argv[], spawn_mode_t mode)
{
    return invoke_syscall(SYSCALL_EXEC, (long)file_name, (long)argc, (long)argv, (long)mode);
}

void sys_exit(void)
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, (long)time, IGNORE, IGNORE, IGNORE);
}

int sys_kill(pid_t pid)
{
    return invoke_syscall(SYSCALL_KILL, (long)pid, IGNORE, IGNORE, IGNORE);
}

int sys_waitpid(pid_t pid)
{
    return invoke_syscall(SYSCALL_WAITPID, (long)pid, IGNORE, IGNORE, IGNORE);
}

void sys_process_show(char* buffer)
{
    invoke_syscall(SYSCALL_PS, (long)buffer, IGNORE, IGNORE, IGNORE);
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
    return invoke_syscall(SYSCALL_TASKSET, (long)pid, (long)mask, IGNORE, IGNORE);
}

void sys_show_exec(int* print_location_y)
{
    invoke_syscall(SYSCALL_SHOW_EXEC, (long)print_location_y, IGNORE, IGNORE, IGNORE);
}

void sys_futex_wait(volatile uint64_t *val_addr, int binsem_id)
{
    invoke_syscall(SYSCALL_FUTEX_WAIT, (long)val_addr, (long)binsem_id, IGNORE, IGNORE);
}

void sys_futex_wakeup(volatile uint64_t *val_addr, int num_wakeup)
{
    invoke_syscall(SYSCALL_FUTEX_WAKEUP, (long)val_addr, (long)num_wakeup, IGNORE, IGNORE);
}

int sys_binsem_get(long key)
{
    return invoke_syscall(SYSCALL_BINSEM_GET, (long)key, IGNORE, IGNORE, IGNORE);
}

void sys_binsem_op(int binsem_id, int op)
{
    invoke_syscall(SYSCALL_BINSEM_OP, (long)binsem_id, (long)op, IGNORE, IGNORE);
}

void* sys_shmpage_get(int key)
{
    return (void*)invoke_syscall(SYSCALL_SHMPGET, (long)key, IGNORE, IGNORE, IGNORE);
}

void sys_shmpage_dt(void *addr)
{
    invoke_syscall(SYSCALL_SHMPDT, (long)addr, IGNORE, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (long)buff, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, (long)x, (long)y, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_screen_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_CLEAR, (long)line1, (long)line2, IGNORE, IGNORE);
}

void sys_screen_scroll(int line1, int line2)
{
    invoke_syscall(SYSCALL_SCREEN_SCROLL, (long)line1, (long)line2, IGNORE, IGNORE);
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
    return invoke_syscall(SYSCALL_MAILBOX_OPEN, (long)name, IGNORE, IGNORE, IGNORE);
}

void sys_mbox_close(int mailbox)
{
    invoke_syscall(SYSCALL_MAILBOX_CLOSE, (long)mailbox, IGNORE, IGNORE, IGNORE);
}

int sys_mbox_send(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_SEND, (long)mailbox, (long)msg, (long)msg_length, IGNORE);
}

int sys_mbox_recv(int mailbox, void *msg, int msg_length)
{
    return invoke_syscall(SYSCALL_MAILBOX_RECV, (long)mailbox, (long)msg, (long)msg_length, IGNORE);
}

long sys_net_recv(uintptr_t addr, size_t length, int num_packet, size_t* frLength)
{
    return invoke_syscall(SYSCALL_NET_RECV, (long)addr, (long)length, (long)num_packet, (long)frLength);
}

void sys_net_send(uintptr_t addr, size_t length)
{
    invoke_syscall(SYSCALL_NET_SEND, (long)addr, (long)length, IGNORE, IGNORE);
}

void sys_net_irq_mode(int mode)
{
    invoke_syscall(SYSCALL_NET_IRQ_MODE, (long)mode, IGNORE, IGNORE, IGNORE);
}

int sys_mkfs(int mode, int* print_location_y)
{
    return invoke_syscall(SYSCALL_MKFS, (long)mode, (long)print_location_y, IGNORE, IGNORE);
}
void sys_statfs(int* print_location_y)
{
    invoke_syscall(SYSCALL_STATFS, (long)print_location_y, IGNORE, IGNORE, IGNORE);
}
int sys_mkdir(char* dirname)
{
    return invoke_syscall(SYSCALL_MKDIR, (long)dirname, IGNORE, IGNORE, IGNORE);
}
int sys_rmdir(char* dirname)
{
    return invoke_syscall(SYSCALL_RMDIR, (long)dirname, IGNORE, IGNORE, IGNORE);
}
int sys_ls(char* dirname, int mode, int* print_location_y)
{
    return invoke_syscall(SYSCALL_LS, (long)dirname, (long)mode, (long)print_location_y, IGNORE);
}
int sys_cd(char* dirname)
{
    return invoke_syscall(SYSCALL_CD, (long)dirname, IGNORE, IGNORE, IGNORE);
}

int sys_touch(char* filename)
{
    return invoke_syscall(SYSCALL_TOUCH, (long)filename, IGNORE, IGNORE, IGNORE);
}
int sys_cat(char* filename, char* buffer)
{
    return invoke_syscall(SYSCALL_CAT, (long)filename, (long)buffer, IGNORE, IGNORE);
}
int sys_fopen(char* filename, int access)
{
    return invoke_syscall(SYSCALL_FOPEN, (long)filename, (long)access, IGNORE, IGNORE);
}
int sys_fread(int fd, char* buffer, int size)
{
    return invoke_syscall(SYSCALL_FREAD, (long)fd, (long)buffer, (long)size, IGNORE);
}
int sys_fwrite(int fd, char* buffer, int size)
{
    return invoke_syscall(SYSCALL_FWRITE, (long)fd, (long)buffer, (long)size, IGNORE);
}
void sys_fclose(int fd)
{
    invoke_syscall(SYSCALL_FCLOSE, (long)fd, IGNORE, IGNORE, IGNORE);
}