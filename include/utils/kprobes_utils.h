#ifndef KPROBES_UTILS_H
#define KPROBES_UTILS_H

#include <linux/kprobes.h>

// typedef for kallsyms_lookup_name func ptr
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

extern kallsyms_lookup_name_t _kallsyms_lookup(void);

#endif // KPROBES_UTILS_H
