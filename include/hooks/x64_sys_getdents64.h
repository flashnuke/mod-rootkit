#ifndef GETDENTS64_HOOK_H
#define GETDENTS64_HOOK_H

#include <linux/dirent.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kstrtox.h>
#include <linux/sched/task.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0)
extern asmlinkage long (*orig_getdents64)(unsigned int fd, char __user *dirp, unsigned int count);
extern asmlinkage long hook_getdents64(unsigned int fd, char __user *dirp, unsigned int count);
#else
extern asmlinkage long (*orig_getdents64)(const struct pt_regs *regs);
extern asmlinkage int hook_getdents64(const struct pt_regs *regs);
#endif

extern long hook_getdents64_impl(unsigned int fd, char __user *dirp, long ret);

#endif // GETDENTS64_HOOK_H
