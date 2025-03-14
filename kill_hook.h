#ifndef KILL_HOOK_H
#define KILL_HOOK_H
// TODO go over code, refine it, remove includes, etc...

#include <linux/linkage.h>
#include <linux/init.h>

#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <asm/unistd.h>

#include "helpers.h" // todo add include protection in header


// TODO dont forget PTREGS macro
static asmlinkage long (*orig_kill)(const struct pt_regs*);

asmlinkage int hook_kill(const struct pt_regs* regs)
{

    /// do some logic before calling original function
    pid_t pid = regs->di;
    int sig = regs->si;
    pr_info("IN PTREGS pid is %d, sig is %d\n", pid, sig);

    return orig_kill(regs);

}


#endif // KILL_HOOK_H
