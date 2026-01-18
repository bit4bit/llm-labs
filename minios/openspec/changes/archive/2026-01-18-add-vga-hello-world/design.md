## Context
Implement basic VGA text mode output for the kernel to verify boot process. This is the first visible output from the kernel and tests Multiboot integration.

## Goals / Non-Goals
- Goals: Display "hola mundo" with rainbow colors
- Non-Goals: Scrolling, cursor management, keyboard input

## Decisions
- **Memory address**: Use 0xB8000 (color VGA text mode)
- **Format**: 2 bytes per character (character byte + attribute byte)
- **Attribute byte**: Bits 0-3 = foreground, bits 4-7 = background
- **Colors**: Standard VGA palette (0=black, 1=blue, 2=green, ..., 15=white)

## Implementation
```c
// VGA memory is mapped at 0xB8000
#define VGA_MEMORY 0xB8000

// Each character cell is 2 bytes
typedef struct {
    char c;
    unsigned char attr;
} vga_cell_t;

// Color constants
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_LIGHT_BROWN   14
#define VGA_COLOR_WHITE         15

// Functions
void vga_putchar(char c, unsigned char color);
void vga_write(const char *str, const unsigned char *colors, size_t len);
void vga_clear(void);
```

## Risks / Trade-offs
- Risk: Writing beyond screen bounds → Mitigation: Only 10 chars, fits on screen
- Risk: No cursor handling → Acceptable for minimal test

## Open Questions
- None
