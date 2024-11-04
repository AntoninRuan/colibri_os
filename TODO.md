# TODO List

## IO

### Keyboard input

- [ ] Input via PS/2
- [ ] Implement USB keyboard support

### Terminal display

- [x] Render bitmap font
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
- [ ] Setting paging + memory allocator

## Misc

- [x] Support multiboot2
- [ ] Basic logging (maybe over serial port)
- [ ] Add support for x86_64
