#include <linux/sched/task.h>  
#include <linux/mm.h>          
#include <linux/uaccess.h>     
#include <linux/rcupdate.h>    
#include "utils/proc_utils.h"

// Reads the command line of a process by its PID
char *read_cmdline_from_task(pid_t pid, size_t *out_size)
{
    struct task_struct *task;
    struct mm_struct *mm;
    char *buffer;
    size_t size;
    ssize_t nread;

    // Locate the target process
    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task) {
        rcu_read_unlock();
        return NULL;
    }
    mm = get_task_mm(task);
    rcu_read_unlock();
    if (!mm)
        return NULL;

    // Lock the memory map
    down_read(&mm->mmap_lock);
    size = mm->arg_end - mm->arg_start;
    up_read(&mm->mmap_lock);

    // Allocate buffer (+1 for null termination)
    buffer = kmalloc(size + 1, GFP_KERNEL);
    if (!buffer) {
        mmput(mm);
        return NULL;
    }

    // Read the process cmdline
    nread = access_process_vm(task, mm->arg_start, buffer, size, 0);
    mmput(mm);
    if (nread < 0 || nread != size) {
        kfree(buffer);
        return NULL;
    }

    buffer[size] = '\0';  // Null-terminate
    if (out_size)
        *out_size = size;
    return buffer;
}

