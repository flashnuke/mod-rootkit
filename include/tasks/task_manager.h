#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <linux/kernel.h>
#include <linux/types.h> // for size_t
#include <linux/kthread.h>
#include <linux/err.h>

typedef int (*task_ptr)(void* data);

// Structure representing a task
struct k_task {
    task_ptr task_func_ptr;
    struct task_struct* task_thread;
};

extern bool thread_is_alive(struct task_struct *p);
extern int run_tasks(struct k_task* tasks, size_t count);
extern void stop_tasks(struct k_task* tasks, size_t count);

#endif // TASK_MANAGER_H
