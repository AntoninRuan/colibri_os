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

- [x] Make line goes up one row each rather than going back to the top
- [x] Support deletion of newline

### File system

- [x] Parsing ELF files
- [ ] VFS
- [ ] Driver for fat32
- [ ] Driver for ext4

## Scheduling

- [x] Implement a scheduler
- [ ] Implement priority based scheduler
- [ ] Threads

## Userspace

- [x] Start userspace programs
- [ ] Memory barrier in programs address space
- [ ] Basic libc
- [ ] Syscalls

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
- [x] TSS implementation for hardware int when in user mode

## SMP

- [x] Detection and init of other cores
- [ ] TSS adaptation for SMP
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
