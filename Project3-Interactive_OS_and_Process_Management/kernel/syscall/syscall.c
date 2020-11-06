#include <os/syscall.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t stval, uint64_t cause)
{
    // syscall[fn](arg1, arg2, arg3)
    // trick: some syscall may modify sepc again
    regs->sepc = regs->sepc + 4;  
    regs->regs[10] = syscall[regs->regs[17]](regs->regs[10], regs->regs[11], regs->regs[12], regs);
}