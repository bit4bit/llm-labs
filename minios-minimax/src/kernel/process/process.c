#include "process.h"
#include "../kernel.h"
#include "../minios.h"
#include "../cpu/tss.h"
#include "../cpu/constants.h"
#include "../cpu/idt.h"
#include "../memory/memory.h"
#include "../debug.h"

process_table_t process_table;
pcb_t* current_process;
volatile int all_processes_exited = 0;

/* Per-process kernel stacks: 4KB each, 4KB-aligned, in BSS */
static char kernel_stacks[MAX_PROCESSES][4096] __attribute__((aligned(4096)));

/* Page directory (defined in boot assembly, used for PDE writes) */
extern uint32_t page_dir[1024];

extern void scheduler_switch(pcb_t* prev, pcb_t* next);
extern void trampoline_to_user(void);

void process_init(void) {
    DEBUG_PROC("Initializing...");
    process_table.count = 0;
    process_table.next_pid = 1;
    process_table.running = 0xFFFFFFFF;
    current_process = (void*)0;
    DEBUG_PROC("Initialized");
}

pcb_t* process_create(const char* name, uint32_t entry_addr) {
    (void)entry_addr; /* Entry is computed from process index */
    if (process_table.count >= MAX_PROCESSES) {
        DEBUG_ERROR("max processes reached");
        return (void*)0;
    }

    uint32_t idx = process_table.count;
    pcb_t* pcb = &process_table.processes[idx];

    pcb->id = process_table.next_pid++;
    pcb->state = PROC_READY;
    pcb->run_count = 0;

    /* Copy name */
    int i = 0;
    while (name[i] && i < 31) {
        pcb->name[i] = name[i];
        i++;
    }
    pcb->name[i] = '\0';

    /* --- Per-process memory regions (task 2.1, 2.3) --- */

    /* Allocate code region: PDE 256+idx */
    void* code_phys = pmm_alloc_frame();
    if (!code_phys) {
        DEBUG_ERROR("Failed to allocate code frame for %s", name);
        return (void*)0;
    }
    page_dir[USER_CODE_PDE(idx)] = ((uint32_t)code_phys) | PDE_USER_4MB;
    DEBUG_PROC("PDE %u: code 0x%X -> phys 0x%X", USER_CODE_PDE(idx),
               USER_CODE_VADDR(idx), (uint32_t)code_phys);

    /* Allocate stack region: PDE 767-idx */
    void* stack_phys = pmm_alloc_frame();
    if (!stack_phys) {
        DEBUG_ERROR("Failed to allocate stack frame for %s", name);
        return (void*)0;
    }
    page_dir[USER_STACK_PDE(idx)] = ((uint32_t)stack_phys) | PDE_USER_4MB;
    DEBUG_PROC("PDE %u: stack 0x%X -> phys 0x%X", USER_STACK_PDE(idx),
               USER_STACK_VADDR(idx), (uint32_t)stack_phys);

    /* Flush TLB after PDE changes */
    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );

    /* Set entry and user stack for this process's memory region */
    pcb->entry = USER_CODE_VADDR(idx);
    pcb->user_stack = USER_STACK_INITIAL(idx);

    /* --- Kernel stack setup (task 1.2, 1.3) --- */
    pcb->kernel_stack_top = (uint32_t)&kernel_stacks[idx][4096];

    /* --- Fake interrupt frame (task 3.1) --- */
    /*
     * Build a synthetic interrupt frame on the kernel stack so the process
     * can be started through scheduler_switch's `ret` -> trampoline_to_user
     * -> pop segs + popal + iret, same as resumed processes go through
     * timer_handler_asm's epilogue.
     *
     * Layout (stack grows down):
     *   SS, ESP, EFLAGS, CS, EIP      (iret frame)
     *   EAX..EDI                       (pushal order)
     *   DS, ES, FS, GS                (segment regs)
     *   &trampoline_to_user           (return address for scheduler_switch ret)
     *   <- kernel_esp points here
     */
    uint32_t* sp = (uint32_t*)pcb->kernel_stack_top;

    /* iret frame (pushed first = highest addresses) */
    *(--sp) = 0x23;                 /* SS: user data selector */
    *(--sp) = pcb->user_stack;      /* ESP: user stack pointer */
    *(--sp) = 0x202;                /* EFLAGS: IF=1, reserved bit 1 */
    *(--sp) = 0x1B;                 /* CS: user code selector */
    *(--sp) = pcb->entry;           /* EIP: entry point */

    /* pushal frame (order: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI) */
    *(--sp) = 0;    /* EAX */
    *(--sp) = 0;    /* ECX */
    *(--sp) = 0;    /* EDX */
    *(--sp) = 0;    /* EBX */
    *(--sp) = 0;    /* ESP (ignored by popal) */
    *(--sp) = 0;    /* EBP */
    *(--sp) = 0;    /* ESI */
    *(--sp) = 0;    /* EDI */

    /* Segment registers */
    *(--sp) = 0x23; /* DS */
    *(--sp) = 0x23; /* ES */
    *(--sp) = 0x23; /* FS */
    *(--sp) = 0x23; /* GS */

    /* Return address for scheduler_switch's `ret` instruction */
    *(--sp) = (uint32_t)trampoline_to_user;

    pcb->kernel_esp = (uint32_t)sp;

    process_table.count++;

    DEBUG_PROC("Created %s PID %u (entry=0x%X stack=0x%X kesp=0x%X)",
               pcb->name, pcb->id, pcb->entry, pcb->user_stack, pcb->kernel_esp);

    return pcb;
}

int process_load(pcb_t* pcb, const uint8_t* binary, uint32_t size) {
    if (pcb == (void*)0 || binary == (void*)0 || size == 0) {
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

    /* Mark prev as READY if it was running (not exited) */
    if (prev != (void*)0 && prev->state != PROC_EXITED) {
        prev->state = PROC_READY;
    }

    /* Find next READY process (round-robin) */
    pcb_t* next = (void*)0;
    uint32_t start_idx = 0;

    if (prev != (void*)0) {
        /* Find index of prev */
        for (uint32_t i = 0; i < process_table.count; i++) {
            if (&process_table.processes[i] == prev) {
                start_idx = i;
                break;
            }
        }
        /* Search starting from next slot */
        for (uint32_t i = 1; i <= process_table.count; i++) {
            uint32_t idx = (start_idx + i) % process_table.count;
            pcb_t* pcb = &process_table.processes[idx];
            if (pcb->state == PROC_READY) {
                next = pcb;
                break;
            }
        }
    } else {
        /* No current process - pick first READY one */
        for (uint32_t i = 0; i < process_table.count; i++) {
            if (process_table.processes[i].state == PROC_READY) {
                next = &process_table.processes[i];
                break;
            }
        }
    }

    if (next == (void*)0) {
        if (prev == (void*)0 || prev->state == PROC_EXITED) {
            /* Check if all processes are exited */
            uint32_t exited = 0;
            for (uint32_t i = 0; i < process_table.count; i++) {
                if (process_table.processes[i].state == PROC_EXITED) {
                    exited++;
                }
            }
            if (exited == process_table.count) {
                DEBUG_SCHED("All processes exited");
                current_process = (void*)0;
                all_processes_exited = 1;
                /* Call the exit report directly — we cannot return to
                 * kernel_main because that stack was abandoned at the
                 * first scheduler_switch(NULL, ...). */
                extern void process_exit_return(void);
                process_exit_return();
                /* process_exit_return does not return */
            }
        }
        /* No other process ready, keep running prev */
        if (prev != (void*)0 && prev->state == PROC_READY) {
            next = prev;
        } else {
            return;
        }
    }

    DEBUG_SCHED("Switching from PID %u to PID %u",
                prev ? prev->id : 0, next->id);

    current_process = next;
    next->state = PROC_RUNNING;

    tss_set_stack(next->kernel_stack_top);

    if (next != prev) {
        scheduler_switch(prev, next);
    }
    /* If next == prev, return normally - timer_handler_asm does pop+iret */
}
