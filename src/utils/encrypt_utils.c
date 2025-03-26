#include "utils/encrypt_utils.h"

void xor_decrypt(char* s) {
    while (*s != '\0') {
        *s ^= XOR_KEY;
        s++;
    }
}
