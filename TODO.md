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
- [ ] Setup IO APIC

## Memory

- [x] Setup a GDT
- [x] Setup paging
- [ ] Add page allocator

## Misc

- [x] Support multiboot2
- [x] Replace gcc with clang for easier cross compiling
- [ ] Basic logging (maybe over serial port)
- [x] Add support for x86_64
