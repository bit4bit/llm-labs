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
    uint32_t eip;
    char name[32];
    uint32_t esp;
    uint32_t ebp;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint16_t ds;
    uint16_t es;
    uint16_t fs;
    uint16_t gs;
    uint32_t run_count;
} pcb_t;

_Static_assert(sizeof(pcb_t) == 92, "C18: pcb_t must be 92 bytes");

typedef struct {
    pcb_t processes[MAX_PROCESSES];
    uint32_t count;
    uint32_t next_pid;
    uint32_t running;
} process_table_t;

_Static_assert(sizeof(process_table_t) == 380, "C18: process_table_t must be 380 bytes");

extern process_table_t process_table;
extern pcb_t* current_process;

void process_init(void);
pcb_t* process_create(const char* name, uint32_t entry_addr);
int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size);
void process_start(pcb_t* pcb);
pcb_t* process_get_current(void);
void process_set_running(uint32_t pid);
void process_mark_exited(pcb_t* pcb);

#endif