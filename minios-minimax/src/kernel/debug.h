#ifndef DEBUG_H
#define DEBUG_H

#include "serial.h"

void debug_print(const char* fmt, ...);

#define DEBUG_INFO(fmt, ...)    debug_print("[INFO]  " fmt "\n", ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)     debug_print("[WARN]  " fmt "\n", ##__VA_ARGS__)
#define DEBUG_ERROR(fmt, ...)    debug_print("[ERROR] " fmt "\n", ##__VA_ARGS__)

#define DEBUG_PMM(fmt, ...)      debug_print("[PMM]   " fmt "\n", ##__VA_ARGS__)
#define DEBUG_VMM(fmt, ...)      debug_print("[VMM]   " fmt "\n", ##__VA_ARGS__)
#define DEBUG_PAGING(fmt, ...)   debug_print("[PAGING]" fmt "\n", ##__VA_ARGS__)
#define DEBUG_PROC(fmt, ...)     debug_print("[PROC]  " fmt "\n", ##__VA_ARGS__)
#define DEBUG_SYSCALL(fmt, ...)  debug_print("[SYSCALL]" fmt "\n", ##__VA_ARGS__)
#define DEBUG_EXCEPT(fmt, ...)   debug_print("[EXCEPT]" fmt "\n", ##__VA_ARGS__)
#define DEBUG_PIT(fmt, ...)      debug_print("[PIT]   " fmt "\n", ##__VA_ARGS__)
#define DEBUG_TIMER(fmt, ...)    debug_print("[TIMER] " fmt "\n", ##__VA_ARGS__)
#define DEBUG_SCHED(fmt, ...)    debug_print("[SCHED] " fmt "\n", ##__VA_ARGS__)

#endif
