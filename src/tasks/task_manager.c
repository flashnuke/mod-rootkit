#include "tasks/task_manager.h"

// run multiple tasks
int run_tasks(k_task* tasks, size_t count) {
    int err;
    size_t i;

    for (i = 0; i < count; i++) {
        char task_nickname[20];
        sprintf(param_name, "task_%d", i);
        struct task_struct* task = kthread_run(tasks[i], NULL, task_nickname);

        if (IS_ERR(task)) {
            return err;
        } else {
            tasks[i]->task_thread = task;
        }

    }
    return 0;
}

// stop tasks
void stop_tasks(k_task* tasks, size_t count) {
    size_t i;
    for (i = 0; i < count; i++) {
        if (tasks[i]->task_thread) {
            kthread_stop(task_thread);
            tasks[i]->task_thread = NULL;
        }
    }
}

