#ifndef FTRACE_UTILS_H
#define FTRACE_UTILS_H

#include <linux/ftrace.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/kallsyms.h>
#include "kprobes_utils.h"

// Prevent recursive loops when hooking
#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

// Macro to define a hook entry
#define HOOK(_name, _hook, _orig) { \
    .name = (_name),                \
    .function = (_hook),            \
    .original = (_orig),            \
}

// Structure representing an ftrace hook
struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

// Function declarations (use `extern` since they are defined in `ftrace_utils.c`)
extern int fh_resolve_hook_address(struct ftrace_hook *hook);
extern int fh_install_hook(struct ftrace_hook *hook);
extern void fh_remove_hook(struct ftrace_hook *hook);
extern int fh_install_hooks(struct ftrace_hook *hooks, size_t count);
extern void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);

#endif // FTRACE_UTILS_H

