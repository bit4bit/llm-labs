#include "process.h"
#include "../kernel.h"
#include "../cpu/tss.h"
#include "../cpu/constants.h"
#include "../cpu/idt.h"

process_table_t process_table;
pcb_t* current_process;

extern void enter_user_mode(uint32_t entry, uint32_t stack);

void process_init(void) {
    serial_print("Process: Initializing...\n");
    process_table.count = 0;
    process_table.next_pid = 1;
    process_table.running = 0xFFFFFFFF;
    serial_print("Process: Initialized\n");
}

pcb_t* process_create(const char* name, uint32_t entry_addr) {
    if (process_table.count >= MAX_PROCESSES) {
        serial_print("Process: Error - max processes reached\n");
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

    serial_print("Process: Created ");
    serial_print(pcb->name);
    serial_print(" with PID ");
    serial_print_uint(pcb->id);
    serial_print("\n");

    return pcb;
}

pcb_t* process_get_current(void) {
    return current_process;
}

void process_start(pcb_t* pcb) {
    serial_print("Process: Starting ");
    serial_print(pcb->name);
    serial_print(" at 0x");
    serial_print_uint(pcb->entry);
    serial_print("\n");

    current_process = pcb;

    uint32_t user_stack = 0xBFFFF000 - 4096;

    tss_set_stack(0xBFFFF000);

    serial_print("Process: Switching to user mode...\n");

    enter_user_mode(pcb->entry, user_stack);

    serial_print("Process: Returned to kernel\n");
}
