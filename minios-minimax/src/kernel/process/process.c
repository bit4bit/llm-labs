#include "process.h"
#include "../kernel.h"
#include "../minios.h"
#include "../cpu/tss.h"
#include "../cpu/constants.h"
#include "../cpu/idt.h"
#include "../debug.h"

process_table_t process_table;
pcb_t* current_process;
volatile int all_processes_exited = 0;

extern void enter_user_mode(uint32_t entry, uint32_t stack);
extern void scheduler_switch(pcb_t* next);

void process_init(void) {
    DEBUG_PROC("Initializing...");
    process_table.count = 0;
    process_table.next_pid = 1;
    process_table.running = 0xFFFFFFFF;
    DEBUG_PROC("Initialized");
}

pcb_t* process_create(const char* name, uint32_t entry_addr) {
    if (process_table.count >= MAX_PROCESSES) {
        DEBUG_ERROR("max processes reached");
        return NULL;
    }

    pcb_t* pcb = &process_table.processes[process_table.count];

    pcb->id = process_table.next_pid++;
    pcb->state = PROC_READY;
    pcb->entry = entry_addr;
    pcb->eip = 0;
    pcb->esp = 0;
    pcb->ebp = 0;
    pcb->eax = 0;
    pcb->ecx = 0;
    pcb->edx = 0;
    pcb->ebx = 0;
    pcb->esi = 0;
    pcb->edi = 0;
    pcb->ds = 0;
    pcb->es = 0;
    pcb->fs = 0;
    pcb->gs = 0;
    pcb->run_count = 0;

    int i = 0;
    while (name[i] && i < 31) {
        pcb->name[i] = name[i];
        i++;
    }
    pcb->name[i] = '\0';

    process_table.count++;

    DEBUG_PROC("Created %s with PID %u", pcb->name, pcb->id);

    return pcb;
}

int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size) {
    if (pcb == NULL || binary == NULL || size == 0) {
        DEBUG_ERROR("invalid load parameters");
        return -1;
    }

    DEBUG_PROC("Loading %s (%u bytes) to 0x%X...", pcb->name, size, pcb->entry);

    uint8_t* dest = (uint8_t*)pcb->entry;
    for (uint32_t i = 0; i < size; i++) {
        dest[i] = binary[i];
    }

    DEBUG_PROC("Binary loaded successfully");

    DEBUG_PROC("First 4 bytes = 0x%X 0x%X 0x%X 0x%X", dest[0], dest[1], dest[2], dest[3]);

    return 0;
}

pcb_t* process_get_current(void) {
    return current_process;
}

void process_set_running(uint32_t pid) {
    process_table.running = pid;
}

void process_mark_exited(pcb_t* pcb) {
    if (pcb) {
        pcb->state = PROC_EXITED;
    }
    process_table.running = 0;
}

void scheduler(void) {
    pcb_t* prev = current_process;

    if (prev == NULL) {
        return;
    }

    if (prev->state != PROC_EXITED) {
        prev->state = PROC_READY;
    }

    pcb_t* next = NULL;
    uint32_t start_idx = 0;
    for (uint32_t i = 0; i < process_table.count; i++) {
        if (&process_table.processes[i] == prev) {
            start_idx = i;
            break;
        }
    }

    for (uint32_t i = 1; i <= process_table.count; i++) {
        uint32_t idx = (start_idx + i) % process_table.count;
        pcb_t* pcb = &process_table.processes[idx];
        if (pcb->state == PROC_READY && pcb != prev) {
            next = pcb;
            break;
        }
    }

    if (next == NULL) {
        if (prev->state == PROC_EXITED) {
            DEBUG_SCHED("All processes exited");
            current_process = NULL;
            all_processes_exited = 1;
            return;
        }
        next = prev;
    }

    DEBUG_SCHED("Switching from PID %u to PID %u", prev ? prev->id : 0, next->id);

    current_process = next;
    next->state = PROC_RUNNING;

    tss_set_stack(KERNEL_STACK_USER_MODE);

    if (next->eip == 0 || next->eip == 1) {
        next->eip = 2;
        enter_user_mode(next->entry, USER_STACK_INITIAL);
    } else {
        scheduler_switch(next);
    }
}

void process_start(pcb_t* pcb) {
    DEBUG_PROC("Starting %s at 0x%X", pcb->name, pcb->entry);

    current_process = pcb;
    pcb->state = PROC_RUNNING;

    uint32_t user_stack = USER_STACK_INITIAL;

    tss_set_stack(KERNEL_STACK_USER_MODE);

    DEBUG_PROC("Switching to user mode...");
    DEBUG_PROC("  entry_point = 0x%X", pcb->entry);
    DEBUG_PROC("  user_stack = 0x%X", user_stack);
    DEBUG_PROC("  tss.esp0 = 0x%X", KERNEL_STACK_USER_MODE);

    volatile uint8_t* code = (volatile uint8_t*)pcb->entry;
    DEBUG_PROC("  code[0..3] = 0x%X 0x%X 0x%X 0x%X", code[0], code[1], code[2], code[3]);

    DEBUG_PROC("Calling enter_user_mode()...");

    enter_user_mode(pcb->entry, user_stack);

    DEBUG_PROC("Returned to kernel");
}
