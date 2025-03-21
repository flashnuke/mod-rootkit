#ifndef GETDENTS_HOOK_H
#define GETDENTS_HOOK_H

#include <linux/dirent.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/kstrtox.h>
#include <linux/mm.h>
#include <linux/sched/task.h>
#include <linux/printk.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0)
extern asmlinkage long (*orig_getdents)(unsigned int fd, char __user *dirp, unsigned int count);
extern asmlinkage long hook_getdents(unsigned int fd, char __user *dirp, unsigned int count);
#else
extern asmlinkage long (*orig_getdents)(const struct pt_regs *regs);
extern asmlinkage int hook_getdents(const struct pt_regs *regs);
#endif

extern long hook_getdents_impl(unsigned int fd, char __user *dirp, long ret);

#endif // GETDENTS_HOOK_H
