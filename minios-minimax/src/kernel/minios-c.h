#ifndef MINIOS_C_H
#define MINIOS_C_H

#include <stddef.h>

/**
 * memset - Fill a block of memory with a specified byte
 * @ptr: Pointer to the block of memory to fill
 * @value: Value to be set (as an unsigned char)
 * @num: Number of bytes to be set to the value
 * 
 * Returns: Pointer to the block of memory
 */
void* memset(void* ptr, int value, size_t num);

#endif /* MINIOS_C_H */