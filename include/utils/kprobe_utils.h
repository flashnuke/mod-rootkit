#ifndef KPROBE_UTILS_H
#define KPROBE_UTILS_H

#include <linux/kprobes.h>

// Typedef for kallsyms_lookup_name function pointer
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

// Function declaration (use extern since it is defined in `kprobe_utils.c`)
extern kallsyms_lookup_name_t _kallsyms_lookup(void);

#endif // KPROBE_UTILS_H
