#ifndef READ_HOOK_H
#define READ_HOOK_H

#include <linux/string.h>   
#include <linux/fs.h> 
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0)
extern asmlinkage long (*orig_read)(unsigned int fd, char __user *buf, size_t count);
extern asmlinkage long hook_read(unsigned int fd, char __user *buf, size_t count);
#else
extern asmlinkage long (*orig_read)(const struct pt_regs *regs);
extern asmlinkage int hook_read(const struct pt_regs *regs);
#endif

extern long hook_read_impl(unsigned int fd, char __user *buf, long ret);

#endif // READ_HOOK_H

