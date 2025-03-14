#ifndef PROC_HIDER_H
#define PROC_HIDER_H


#include <linux/mm.h>         // For `get_cmdline`

#include <linux/fs.h>
#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/fdtable.h>    // For struct file


#include <linux/dirent.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/kprobes.h>
#include <linux/module.h>6
#include <linux/kernel.h>6
#include <linux/ftrace.h>6
#include <asm/unistd.h>
#include <linux/printk.h>

#include <linux/fs.h>      // for struct file and struct dir_context


#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/types.h>


#include <linux/sched/task.h>  // For get_task_comm()


/*
 * Function: check_cmdline_contains_sensitive
 * --------------------------------------------
 * Accepts a PID string (in decimal) and checks whether the process's
 * command-line (as stored in its mm struct, between arg_start and arg_end)
 * contains the substring "SENSITIVE".
 *
 * Returns:
 *   true if the cmdline contains "SENSITIVE", false otherwise.
 */
static bool check_cmdline_contains_sensitive(const char *pid_str)
{    

    struct task_struct *task;
    pid_t pid;
    char comm[TASK_COMM_LEN];

    // Convert pid_str to integer
    if (kstrtoint(pid_str, 10, &pid)) {
        pr_err("Invalid PID: %s\n", pid_str);
        return false;
    }

    // Find the task using pid_task() instead of find_task_by_vpid()
    rcu_read_lock();
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (task) {
        get_task_comm(comm, task);
        pr_info("Process %d command: %s\n", pid, comm);
    } else {
        pr_err("Failed to find task for PID %d\n", pid);
    }
    rcu_read_unlock();
    return true;

}

static char *extract_pid_from_proc_path(const char *path)
{
    const char *start, *end;
    size_t len;
    char *pid_str;

    /* Ensure the path starts with "/proc/" */
    if (strncmp(path, "/proc/", 6) != 0)
        return NULL;

    start = path + 6;  // Move past "/proc/"
    end = strchr(start, '/'); // Find the next '/' after the PID

    if (!end)
        return NULL; // If there's no '/', the path is invalid

    len = end - start; // Compute PID length
    if (len == 0 || len >= 16) // Sanity check (PIDs shouldn't be too long)
        return NULL;

    pid_str = kmalloc(len + 1, GFP_KERNEL);
    if (!pid_str)
        return NULL;

    strncpy(pid_str, start, len);
    pid_str[len] = '\0'; // Null-terminate the string

    return pid_str; // Caller must free() this
}

#endif /* PROC_HIDER_H */
