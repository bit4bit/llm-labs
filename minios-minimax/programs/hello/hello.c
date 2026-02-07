#include "../lib/syscall.h"

void _start(void) {
    const char* msg = "Hello from C program!\n";
    
    /* Calculate string length manually */
    uint32_t len = 0;
    while (msg[len]) {
        len++;
    }
    
    /* Write message to stdout (fd = 1) */
    write(1, msg, len);
    
    /* Exit cleanly */
    exit(0);
}