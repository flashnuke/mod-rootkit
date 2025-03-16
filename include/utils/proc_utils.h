#ifndef PROC_UTILS_H
#define PROC_UTILS_H

#include <linux/types.h>
#include <linux/slab.h>

extern char *read_cmdline_from_task(pid_t pid, size_t *out_size);

#endif // PROC_UTILS_H

