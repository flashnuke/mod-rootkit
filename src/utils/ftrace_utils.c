#include "utils/ftrace_utils.h"

// Resolve the address of the function being hooked
int fh_resolve_hook_address(struct ftrace_hook *hook) {
    kallsyms_lookup_name_t kallsyms_lookup_name = _kallsyms_lookup();
    hook->address = kallsyms_lookup_name(hook->name);

    if (!hook->address) {
        return -ENOENT;
    }

#if USE_FENTRY_OFFSET
    *((unsigned long *)hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long *)hook->original) = hook->address;
#endif

    return 0;
}

// Notrace function to modify execution flow
void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                             struct ftrace_ops *ops, struct ftrace_regs *regs) {
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    regs->ip = (unsigned long)hook->function;
#else
    if (!within_module(parent_ip, THIS_MODULE)) {
        ((struct pt_regs *)regs)->ip = (unsigned long)hook->function;
    }
#endif
}

// Install a single hook
int fh_install_hook(struct ftrace_hook *hook) {
    int err;

    err = fh_resolve_hook_address(hook);
    if (err) {
        return err;
    }

    hook->ops.func = (ftrace_func_t)fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS
                      | FTRACE_OPS_FL_RECURSION_SAFE
                      | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        return err;
    }

    return 0;
}

// Remove a single hook
void fh_remove_hook(struct ftrace_hook *hook) {
    int err;
    err = unregister_ftrace_function(&hook->ops);
    if (err) {
        return;
    }
    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        return;
    }
}

// Install multiple hooks
int fh_install_hooks(struct ftrace_hook *hooks, size_t count) {
    int err;
    size_t i;

    for (i = 0; i < count; i++) {
        err = fh_install_hook(&hooks[i]);
        if (err)
            goto error;
    }
    return 0;

error:
    while (i != 0) {
        fh_remove_hook(&hooks[--i]);
    }
    return err;
}

// Remove multiple hooks
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count) {
    size_t i;
    for (i = 0; i < count; i++) {
        fh_remove_hook(&hooks[i]);
    }
}

