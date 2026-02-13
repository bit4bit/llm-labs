#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

void pit_init(void);
void timer_handler(void);
uint32_t pit_get_ticks(void);

#endif
