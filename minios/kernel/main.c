void kernel_main(unsigned int magic, void *mb_info) {
    volatile unsigned short *vga = (unsigned short *)0xB8000;
    const char *msg = "MiniOS booted!";
    for (int i = 0; msg[i] != '\0'; i++) {
        vga[i] = (0x0F << 8) | msg[i];
    }
    while (1) {
        __asm__ volatile ("hlt");
    }
}
