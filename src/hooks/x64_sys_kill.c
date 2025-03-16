#include "hooks/x64_sys_kill.h"

// Pointer to the original kill syscall
asmlinkage long (*orig_kill)(const struct pt_regs *);

// Hook function for kill syscall
asmlinkage int hook_kill(const struct pt_regs *regs)
{
    pid_t pid = regs->di;
    int sig = regs->si;

    pr_info("IN PTREGS pid is %d, sig is %d\n", pid, sig);

    return orig_kill(regs);
}

