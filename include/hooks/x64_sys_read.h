#ifndef READ_HOOK_H
#define READ_HOOK_H

#include <linux/dirent.h>
#include <linux/ptrace.h>   
#include <linux/slab.h>     
#include <linux/uaccess.h>  
#include <linux/string.h>   
#include <linux/kstrtox.h>  
#include <linux/mm.h>       
#include <linux/sched/task.h>
#include <linux/printk.h>   
#include <linux/fs.h> // TODO clean include

extern asmlinkage long (*orig_read)(const struct pt_regs *);

extern asmlinkage int hook_read(const struct pt_regs *regs);

#endif // READ_HOOK_H

