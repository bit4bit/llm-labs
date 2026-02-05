## ADDED Requirements

### Requirement: Multiboot header
The kernel SHALL contain a valid Multiboot header within the first 8192 bytes, aligned to 4-byte boundary, with magic value 0x1BADB002.

#### Scenario: GRUB loads kernel
- **WHEN** BIOS loads GRUB and GRUB finds the Multiboot header
- **THEN** GRUB loads kernel.bin into memory and transfers control to the kernel entry point

### Requirement: Protected mode entry
The kernel entry point SHALL be entered in 32-bit protected mode with CS set to a flat code segment.

#### Scenario: 32-bit execution
- **WHEN** GRUB transfers control to kernel
- **THEN** the processor is in protected mode, 32-bit operation is enabled, and CS descriptor has base 0 and limit 0xFFFFFFFF

### Requirement: Text mode VGA output
The kernel SHALL print the string "MinOS Loaded" to the VGA text buffer at physical address 0xB8000.

#### Scenario: VGA text display
- **WHEN** kernel entry point is reached
- **THEN** the kernel writes each character of "MinOS Loaded" to VGA buffer using color 0x07 (light gray on black)
- **AND** cursor position is at row 0, column 0

### Requirement: Kernel halts on completion
After displaying the message, the kernel SHALL halt the CPU using the HLT instruction.

#### Scenario: Kernel halt
- **WHEN** "MinOS Loaded" is displayed
- **THEN** kernel enters infinite loop of HLT instructions
