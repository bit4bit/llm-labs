## ADDED Requirements

### Requirement: Timer interrupt initialization
The PIT driver SHALL initialize channel 0 to generate interrupts at a configurable frequency.

#### Scenario: Initialization with default frequency
- **WHEN** `pit_init()` is called with frequency 100 Hz
- **THEN** PIT channel 0 is configured to fire at approximately 100 interrupts per second
- **THEN** IRQ0 is unmasked in the PIC

#### Scenario: Initialization with custom frequency
- **WHEN** `pit_init(frequency)` is called with frequency F
- **THEN** PIT divisor is set to (1193182 / F)
- **THEN** mode 3 (square wave generator) is selected

### Requirement: Timer interrupt handler
The PIT driver SHALL provide an interrupt handler that acknowledges the interrupt and optionally calls a user callback.

#### Scenario: Timer interrupt fires
- **WHEN** PIT generates IRQ0 interrupt
- **THEN** interrupt handler acknowledges PIT via port 0x43
- **THEN** if callback is registered, callback function is invoked

### Requirement: Timer sleep functionality
The kernel SHALL provide a sleep function that blocks the current process for a specified duration.

#### Scenario: Process sleeps for 1 second
- **WHEN** `sleep(seconds)` is called by a running process
- **THEN** process state changes to BLOCKED
- **THEN** process is added to sleep queue with wake time
- **THEN** scheduler selects next ready process
- **THEN** after specified time, process state changes to READY

### Requirement: Timer tick functionality
The kernel SHALL provide a mechanism to register a tick callback invoked on every timer interrupt.

#### Scenario: Tick callback registration
- **WHEN** `pit_set_tick_callback(callback)` is called
- **THEN** callback is stored and invoked on every timer interrupt
- **AND** only one callback can be registered at a time
