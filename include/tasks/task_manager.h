#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>

typedef int (*task_ptr)(void);

// Structure representing a task
struct k_task {
    task_ptr task_func_ptr;
    struct task_struct* task_thread;
};

extern int run_tasks(struct k_task* tasks, size_t count);
extern void stop_tasks(struct k_task* tasks, size_t count);

#endif // TASK_MANAGER_H
