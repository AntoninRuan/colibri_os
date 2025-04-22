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
- [ ] Driver for fat32
- [ ] Driver for ext4

## Scheduling

- [ ] Start userspace programs
- [ ] Implement a scheduler

## Interrupts

- [x] Setup an IDT
- [x] Setup APIC
- [x] Setup IO APIC

## Memory

- [x] Setup a GDT
- [x] Setup paging
- [x] Add page allocator
- [x] Virtual Memory Manager

## CPU Extensions

### x86-64

- [x] Add support for x86_64
- [ ] Support multiprocessor

#### SSE

- [x] Enable SSE support
- [ ] Add #XM handler

## Misc

- [x] Support multiboot2
- [x] Replace gcc with clang for easier cross compiling
- [ ] Basic logging (maybe over serial port)
