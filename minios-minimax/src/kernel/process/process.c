#include "process.h"
#include "../kernel.h"
#include "../minios.h"
#include "../cpu/tss.h"
#include "../cpu/constants.h"
#include "../cpu/idt.h"
#include "../debug.h"

process_table_t process_table;
pcb_t* current_process;

extern void enter_user_mode(uint32_t entry, uint32_t stack);

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
    pcb->state = PROC_RUNNING;
    pcb->entry = entry_addr;
    pcb->eip = 0;

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

void process_start(pcb_t* pcb) {
    DEBUG_PROC("Starting %s at 0x%X", pcb->name, pcb->entry);

    current_process = pcb;

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
