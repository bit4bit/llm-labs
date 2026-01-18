## ADDED Requirements

### Requirement: VGA Text Mode Output
The system SHALL provide functions to write characters to VGA text mode memory at 0xB8000.

#### Scenario: Write single colored character
- **WHEN** `vga_putchar()` is called with a character and color
- **THEN** the character SHALL be written to VGA memory at the current cursor position
- **AND** the attribute byte SHALL set the foreground and background color

#### Scenario: Write string with per-character colors
- **WHEN** `vga_write()` is called with a string and color array
- **THEN** each character SHALL be written with its corresponding color
- **AND** the cursor SHALL advance after each character

### Requirement: Hello World Display
The kernel SHALL display "hola mundo" on screen with each letter in a different color upon boot.

#### Scenario: Rainbow hello world
- **WHEN** the kernel starts
- **THEN** it SHALL print the string "hola mundo" (10 characters)
- **AND** each character SHALL have a different color from 0-9
- **AND** the output SHALL appear at the top-left of the screen (row 0, column 0)

### Requirement: VGA Colors
The system SHALL define standard VGA color constants.

#### Scenario: Color constants defined
- **WHEN** the VGA module is included
- **THEN** it SHALL provide constants for colors 0-15 (black through white)
- **AND** these constants SHALL match standard VGA palette values
