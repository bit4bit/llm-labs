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

    /* Per-process user code and stack PDEs are allocated in process_create() */
}
