#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include "../minios.h"

#define MAX_PROCESSES 4

#define PROC_RUNNING 0
#define PROC_EXITED  1

typedef struct {
    uint32_t id;
    uint32_t state;
    uint32_t entry;
    uint32_t eip;
    char name[32];
} pcb_t;

typedef struct {
    pcb_t processes[MAX_PROCESSES];
    uint32_t count;
    uint32_t next_pid;
    uint32_t running;
} process_table_t;

void process_init(void);
pcb_t* process_create(const char* name, uint32_t entry_addr);
int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size);
void process_start(pcb_t* pcb);
pcb_t* process_get_current(void);
void process_set_running(uint32_t pid);
void process_mark_exited(pcb_t* pcb);

#endif
