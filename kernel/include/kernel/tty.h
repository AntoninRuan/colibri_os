#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <kernel/vga.h>
#include <stddef.h>
#include <stdint.h>

int terminal_initialize(struct framebuffer *fb);
int terminal_clear();
void terminal_return();
void terminal_write(u8 data);
void terminal_writestring(const char *data);

#endif  // KERNEL_TTY_H
