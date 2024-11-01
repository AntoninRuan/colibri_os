#ifndef KERNEL_TTY_H
#define KERNEL_TTY_H

#include <stddef.h>
#include <stdint.h>
#include <kernel/vga.h>

int terminal_initialize(struct framebuffer* fb);
int terminal_clear();
void terminal_return();
void terminal_putchar(uint8_t c, uint16_t color);
void terminal_write(uint8_t data);
void terminal_writestring(const char* data);

#endif // KERNEL_TTY_H
