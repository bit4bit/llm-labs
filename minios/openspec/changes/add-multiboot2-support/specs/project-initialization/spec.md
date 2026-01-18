## MODIFIED Requirements

### Requirement: Multiboot Header
The system SHALL provide a Multiboot header within the first 8192 bytes of the kernel, aligned to the appropriate boundary for the target specification version.

#### Scenario: Multiboot1 header (deprecated)
- **WHEN** the kernel is compiled with Multiboot1 support
- **THEN** the header SHALL contain magic `0x1BADB002`
- **AND** flags SHALL be `0x00010003` for memory info
- **AND** checksum SHALL be `-(magic + flags) % 2^32`
- **AND** the header SHALL be aligned to 4-byte boundary

#### Scenario: Multiboot2 header
- **WHEN** the kernel is compiled with Multiboot2 support
- **THEN** the header SHALL contain magic `0xE85250D6`
- **AND** architecture field SHALL be `4` for x86_64
- **AND** header length SHALL include all required fields
- **AND** checksum SHALL be `-(magic + architecture + header_length) % 2^32`
- **AND** the header SHALL be aligned to 8-byte boundary
- **AND** the header SHALL be within first 8192 bytes of kernel image

### Requirement: Boot Magic Verification
The system SHALL verify the boot magic passed by the bootloader to confirm Multiboot compliance.

#### Scenario: Multiboot1 magic verification
- **WHEN** the kernel receives `0x2BADB002` in EAX
- **THEN** the kernel SHALL use Multiboot1 boot info parsing
- **AND** the boot info pointer in EBX SHALL be cast to `multiboot_info_t*`

#### Scenario: Multiboot2 magic verification
- **WHEN** the kernel receives `0x36D76289` in EAX
- **THEN** the kernel SHALL use Multiboot2 boot info parsing
- **AND** the boot info pointer in EBX SHALL be cast to `multiboot2_boot_info_t*`

### Requirement: Boot Information Parsing
The system SHALL parse boot information tags provided by the bootloader to obtain memory map, boot device, and command line.

#### Scenario: Multiboot1 info structure
- **WHEN** Multiboot1 magic is verified
- **THEN** the kernel SHALL read memory map from `mmap_*` fields
- **AND** boot device from `boot_device` field
- **AND** command line from `cmdline` field

#### Scenario: Multiboot2 tag iteration
- **WHEN** Multiboot2 magic is verified
- **THEN** the kernel SHALL iterate through boot info tags
- **AND** parse memory map from `BT_INFO_MMAP` tag
- **AND** parse boot device from `BT_INFO_BOOTDEV` tag
- **AND** parse command line from `BT_INFO_CMDLINE` tag
- **AND** terminate iteration at `BT_END` tag

### Requirement: Memory Information
The system SHALL provide memory information obtained from the boot bootloader.

#### Scenario: Multiboot1 memory info
- **WHEN** Multiboot1 boot info is available
- **THEN** lower memory SHALL be available from `mem_lower` (typically 640 KB)
- **AND** upper memory SHALL be available from `mem_upper`

#### Scenario: Multiboot2 memory map
- **WHEN** Multiboot2 boot info is available
- **THEN** memory map tag SHALL contain entry size and entry version
- **AND** each entry SHALL contain base address, length, and type
- **AND** type 1 SHALL indicate available memory
- **AND** type 2 SHALL indicate reserved memory

### Requirement: Boot Device Information
The system SHALL identify the boot device from which the kernel was loaded.

#### Scenario: Multiboot1 boot device
- **WHEN** Multiboot1 boot info is available
- **THEN** boot device SHALL be encoded in `boot_device` as BIOS drive number
- **AND** partition information SHALL be available for BIOS partitioning

#### Scenario: Multiboot2 boot device
- **WHEN** Multiboot2 boot info is available
- **THEN** boot device tag SHALL contain BIOS drive number
- **AND** partition sequence SHALL identify the boot partition

### Requirement: GRUB Boot Configuration
The system SHALL be bootable via GRUB bootloader using the appropriate Multiboot directive.

#### Scenario: Multiboot1 boot with GRUB
- **WHEN** GRUB loads the kernel with `multiboot` directive
- **THEN** GRUB SHALL detect Multiboot1 header automatically
- **AND** pass Multiboot1 magic and boot info

#### Scenario: Multiboot2 boot with GRUB
- **WHEN** GRUB loads the kernel with `multiboot2` directive
- **THEN** GRUB SHALL use Multiboot2 protocol
- **AND** pass Multiboot2 magic and boot info
- **AND** the kernel SHALL be a 64-bit ELF executable
