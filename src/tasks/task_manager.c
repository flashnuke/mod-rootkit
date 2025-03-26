#include "tasks/task_manager.h"

// run multiple tasks
int run_tasks(struct k_task* tasks, size_t count) {
    int err;
    size_t i;

    for (i = 0; i < count; i++) {
        char task_nickname[20];
        sprintf(task_nickname, "task_%ld", i);
        struct task_struct* task = kthread_run(tasks[i].task_func_ptr, NULL, task_nickname);

        if (IS_ERR(task)) { // tasks that were created will be closed in module_exit
            return err;
        } else {
            tasks[i].task_thread = task;
        }

    }
    return 0;
}

// stop tasks
void stop_tasks(struct k_task* tasks, size_t count) {
    size_t i;
    for (i = 0; i < count; i++) {
        if (tasks[i].task_thread) {
            kthread_stop(tasks[i].task_thread);
            tasks[i].task_thread = NULL;
        }
    }
}

