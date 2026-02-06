.globl enable_paging_dir
.type enable_paging_dir, @function
enable_paging_dir:
    # Enable CR4.PSE for 4MB pages
    mov %cr4, %eax
    or $0x10, %eax
    mov %eax, %cr4

    # Load CR3 with page directory address (0x5000)
    mov $page_dir, %eax
    mov %eax, %cr3

    # Enable paging (CR0.PG)
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

    ret
