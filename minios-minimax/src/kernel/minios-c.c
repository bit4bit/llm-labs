#include "minios-c.h"
#include <stdint.h>

/**
 * memset - Fill a block of memory with a specified byte (simple implementation)
 * @ptr: Pointer to the block of memory to fill
 * @value: Value to be set (as an unsigned char)
 * @num: Number of bytes to be set to the value
 * 
 * Returns: Pointer to the block of memory
 */
void* memset(void* ptr, int value, size_t num) {
    uint8_t* p = (uint8_t*)ptr;
    uint8_t v = (uint8_t)value;
    
    for (size_t i = 0; i < num; i++) {
        p[i] = v;
    }
    
    return ptr;
}