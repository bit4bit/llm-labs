#include <stdint.h>
#include "../minios.h"
#include "../debug.h"
#include "memory.h"
#include "../minios-c.h"

extern uint32_t page_dir[1024];
extern void enable_paging_dir(void);

void vmm_init(void) {
    DEBUG_VMM("Initializing...");

    memset(page_dir, 0, 1024 * sizeof(uint32_t));

    DEBUG_VMM("Setting up kernel identity mapping (4MB pages)...");

    for (int i = PDE_LOW_MEMORY; i <= PDE_KERNEL_END; i++) {
        page_dir[i] = (i * PAGE_SIZE_4MB) | PDE_KERNEL_4MB;
    }

    DEBUG_VMM("PDE 0 set (user, 0-4MB for VGA)");
    DEBUG_VMM("PDE 1-63 set (kernel, 4-256MB identity)");

    DEBUG_VMM("Enabling...");

    enable_paging_dir();

    DEBUG_VMM("Enabled!");

    DEBUG_VMM("Allocating user program region...");
    void* user_prog_phys = pmm_alloc_frame();
    if (!user_prog_phys) {
        DEBUG_ERROR("Failed to allocate user program region");
        return;
    }
    page_dir[PDE_USER_PROGRAM] = ((uint32_t)user_prog_phys) | PDE_USER_4MB;
    DEBUG_VMM("PDE %u: 0x%X -> 0x%X", PDE_USER_PROGRAM, PDE_INDEX_TO_VADDR(PDE_USER_PROGRAM), (uint32_t)user_prog_phys);

    DEBUG_VMM("Allocating user stack region...");
    void* user_stack_phys = pmm_alloc_frame();
    if (!user_stack_phys) {
        DEBUG_ERROR("Failed to allocate user stack region");
        return;
    }
    page_dir[PDE_USER_STACK] = ((uint32_t)user_stack_phys) | PDE_USER_4MB;
    DEBUG_VMM("PDE %u: 0x%X -> 0x%X", PDE_USER_STACK, PDE_INDEX_TO_VADDR(PDE_USER_STACK), (uint32_t)user_stack_phys);

    DEBUG_VMM("Flushing TLB...");
    __asm__ volatile (
        "movl %%cr3, %%eax\n"
        "movl %%eax, %%cr3\n"
        : : : "eax", "memory"
    );
    DEBUG_VMM("TLB flushed");
}
