#include "process.h"
#include "../kernel.h"
#include "../minios.h"
#include "../cpu/tss.h"
#include "../cpu/constants.h"
#include "../cpu/idt.h"

extern void serial_print(const char* str);
extern void serial_print_uint(uint32_t val);
extern void serial_print_hex(uint32_t val);

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

int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size) {
    if (pcb == NULL || binary == NULL || size == 0) {
        serial_print("Process: Error - invalid load parameters\n");
        return -1;
    }

    serial_print("Process: Loading ");
    serial_print(pcb->name);
    serial_print(" (");
    serial_print_uint(size);
    serial_print(" bytes) to 0x");
    serial_print_hex(pcb->entry);
    serial_print("...\n");

    /* Copy binary to user space address */
    uint8_t* dest = (uint8_t*)pcb->entry;
    for (uint32_t i = 0; i < size; i++) {
        dest[i] = binary[i];
    }

    serial_print("Process: Binary loaded successfully\n");

    /* Verify first few bytes */
    serial_print("Process: First 4 bytes = 0x");
    serial_print_hex(dest[0]);
    serial_print(" 0x");
    serial_print_hex(dest[1]);
    serial_print(" 0x");
    serial_print_hex(dest[2]);
    serial_print(" 0x");
    serial_print_hex(dest[3]);
    serial_print("\n");

    return 0;
}

pcb_t* process_get_current(void) {
    return current_process;
}

void process_start(pcb_t* pcb) {
    serial_print("Process: Starting ");
    serial_print(pcb->name);
    serial_print(" at 0x");
    serial_print_hex(pcb->entry);
    serial_print("\n");

    current_process = pcb;

    uint32_t user_stack = USER_STACK_INITIAL;

    tss_set_stack(KERNEL_STACK_USER_MODE);

    serial_print("Process: Switching to user mode...\n");
    serial_print("Process:   entry_point = 0x");
    serial_print_hex(pcb->entry);
    serial_print("\n");
    serial_print("Process:   user_stack = 0x");
    serial_print_hex(user_stack);
    serial_print("\n");
    serial_print("Process:   tss.esp0 = 0x");
    serial_print_hex(KERNEL_STACK_USER_MODE);
    serial_print("\n");
    
    // Verify the code at entry point
    volatile uint8_t* code = (volatile uint8_t*)pcb->entry;
    serial_print("Process:   code[0..3] = 0x");
    serial_print_hex(code[0]);
    serial_print(" 0x");
    serial_print_hex(code[1]);
    serial_print(" 0x");
    serial_print_hex(code[2]);
    serial_print(" 0x");
    serial_print_hex(code[3]);
    serial_print("\n");
    
    serial_print("Process: Calling enter_user_mode()...\n");

    enter_user_mode(pcb->entry, user_stack);

    serial_print("Process: Returned to kernel\n");
}
