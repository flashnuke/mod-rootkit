#ifndef OPENAT_HOOK_H
#define OPENAT_HOOK_H

#include <linux/ptrace.h>   
#include <linux/uaccess.h>  


extern asmlinkage long (*orig_openat)(const struct pt_regs *);

extern asmlinkage int hook_openat(const struct pt_regs *regs);

#endif // OPENAT_HOOK_H

