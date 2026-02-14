#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "../minios.h"

#define MAX_PROCESSES 4

#define PROC_READY   0
#define PROC_RUNNING 1
#define PROC_EXITED  2

typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t entry;
    char name[32];
    uint32_t kernel_esp;
    uint32_t kernel_stack_top;
    uint32_t user_stack;
    uint32_t run_count;
} pcb_t;

_Static_assert(sizeof(pcb_t) == 60, "C18: pcb_t must be 60 bytes");

/* PCB field offsets for assembly (must match struct layout above) */
#define PCB_OFFSET_KERNEL_ESP       44  /* offsetof(pcb_t, kernel_esp) */
#define PCB_OFFSET_KERNEL_STACK_TOP 48  /* offsetof(pcb_t, kernel_stack_top) */

typedef struct {
    pcb_t processes[MAX_PROCESSES];
    uint32_t count;
    uint32_t next_pid;
    uint32_t running;
} process_table_t;

_Static_assert(sizeof(process_table_t) == 252, "C18: process_table_t must be 252 bytes");

extern process_table_t process_table;
extern pcb_t* current_process;

void process_init(void);
pcb_t* process_create(const char* name, uint32_t entry_addr);
int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size);
pcb_t* process_get_current(void);
void process_set_running(uint32_t pid);
void process_mark_exited(pcb_t* pcb);

#endif
