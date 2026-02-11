#include "../../lib/syscall.h"

void _start(void) {
    const char* msg = "[SELFCHECK] MiniOS user-mode self-check passed!\n";

    uint32_t len = 0;
    while (msg[len]) {
        len++;
    }

    write(1, msg, len);

    exit(0);
}
