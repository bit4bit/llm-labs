#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>

/*
 * programs.h - User program registry
 * 
 * This header provides extern declarations for all embedded user programs.
 * The actual program data is defined in programs.c which includes the
 * generated binary arrays.
 */

/* Hello world program */
extern uint8_t hello_bin[];
extern uint32_t hello_bin_size;

extern uint8_t selfcheck_bin[];
extern uint32_t selfcheck_bin_size;

/* Add more programs here as needed:
 * extern uint8_t shell_bin[];
 * extern uint32_t shell_bin_size;
 */

#endif /* PROGRAMS_H */
