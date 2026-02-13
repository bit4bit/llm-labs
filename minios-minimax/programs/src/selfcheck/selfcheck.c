#include "../../lib/syscall.h"

void _start(void) {
    volatile int counter = 0;
    for (int i = 0; i < 100000000; i++) {
        counter++;
    }
    exit(0);
}
