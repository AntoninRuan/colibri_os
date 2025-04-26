#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/log.h>
#include <kernel/memory/vm.h>
#include <kernel/pc_font.h>
#include <kernel/tty.h>
#include <kernel/vga.h>

extern uint8_t _binary_font_psfu_start;
extern uint8_t _binary_font_psfu_end;

struct framebuffer display = {0};
const struct pc_font* font = (struct pc_font *) &_binary_font_psfu_start;
uint8_t *font_start;
size_t TERMINAL_WIDTH, TERMINAL_HEIGHT;
size_t row, column;
uint16_t fg, bg;

int terminal_initialize(struct framebuffer *fb) {
    memcpy(&display, fb, sizeof(struct framebuffer));
    // display.addr is still a physical address
    // asking to map it to a virtual one
    uint64_t vaddr = (uint64_t) map_mmio(NULL, display.addr, display.pitch * display.height, true);
    if (vaddr == 0) {
        logf(ERROR, "MMIO mapping for terminal buffer failed");
        return 1;
    }
    display.addr = vaddr;

    font_start = (uint8_t *)(&_binary_font_psfu_start + font->headersize);

    TERMINAL_HEIGHT = display.height / (font->height + 1);
    TERMINAL_WIDTH = display.width / (font->width + 1);
    logf(INFO, "Init tty with size %dx%d", TERMINAL_WIDTH, TERMINAL_HEIGHT);
    row = 0;
    column = 0;

    fg = (uint16_t) 0xFFFFFF;
    bg = 0;

    return terminal_clear();
}

int terminal_clear() {
    if (display.addr == 0) {
        return 1;
    }

    memset((char *)display.addr, 0, display.pitch * display.height);
    return 0;
}

// TODO support other bpp than 32
void terminal_putchar(uint8_t c) {
    if (column == TERMINAL_WIDTH)
        terminal_return();

    uint64_t row_offset = (row * (font->height + 1) * display.pitch);
    uint64_t column_offset = (column * (font->width + 1) * (display.bpp / 8));

    uint8_t *font_char = font_start + (c * font->bytesperglyph);

    uint8_t bitmap_offset;
    for (uint32_t y = 0; y < font->height; y ++) {
        bitmap_offset = ceildiv(font->width, 8) * y;

        for (uint32_t x = 0; x < font->width; x += 8) {
            for (uint8_t i = 0; i < 8; i ++) {
                if ((x + i) >= font->width) break;

                uint32_t *index = (uint32_t *) (display.addr + row_offset + column_offset + (x + i) * (display.bpp / 8));

                if (font_char[bitmap_offset] & (0x80 >> i))
                    *index = fg;
                else
                    *index = bg;
            }
            bitmap_offset ++;
        }
        row_offset += display.pitch;
    }

    column ++;
    // if (++ column == TERMINAL_WIDTH) {
    //     terminal_return();
    // }
}

void terminal_return() {
    if (++ row == TERMINAL_HEIGHT) {
        terminal_clear();
        row = 0;
    }
    column = 0;
}

void terminal_backspace() {
    if (column != 0) {
        column --;
        terminal_putchar(' ');
        column --;
    } else if (row != 0) {
        column = TERMINAL_WIDTH;
        row --;
    }
}

void terminal_write(uint8_t data) {
    if (display.addr == 0) {
        return;
    }

    if (0x20 <= data && data < 0x7F) {
        terminal_putchar(data);
        return;
    }

    switch(data) {
        case '\b':
            terminal_backspace();
            break;
        case '\t':
            // TODO add support for \t
            break;
        case '\n':
            terminal_return();
            break;

        default:
            break;
    }
}

void terminal_writestring(const char* data) {
    int i = 0;
    while(data[i]) {
        terminal_write(data[i]);
        i ++;
    }
}
