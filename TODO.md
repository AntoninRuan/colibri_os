# TODO List

## IO

### Keyboard input

- [x] Input via PS/2
- [ ] Handling PS/2 set 2
- [ ] Implement USB keyboard support

### Framebuffer rendering

- [x] Render bitmap font
- [ ] Support bitmap rendering in other bpp than 32

### Terminal display

- [ ] Make line goes up one row each rather than going back to the top
- [ ] Support deletion of newline

### File system

- [ ] Parsing ELF files
- [ ] VFS
- [ ] Driver for fat32
- [ ] Driver for ext4

## Scheduling

- [ ] Implement a scheduler
- [ ] Start userspace programs

## Interrupts

- [x] Setup an IDT
- [x] Setup APIC
- [x] Setup IO APIC

## Memory

- [x] Setup a GDT
- [x] Setup paging
- [x] Add page allocator
- [x] Virtual Memory Manager
- [x] Kernel Heap

## SMP

- [x] Detection and init of other cores
- [ ] Synchronisation (spinlock, semaphore, ...)

## CPU Extensions

### x86-64

- [x] Add support for x86_64

#### SSE

- [x] Enable SSE support
- [ ] Add #XM handler

## Misc

- [x] Support multiboot2
- [x] Replace gcc with clang for easier cross compiling
- [x] Basic logging (maybe over serial port)

### Timer
- [x] Setup HPET
- [x] Setup LAPIC timer
