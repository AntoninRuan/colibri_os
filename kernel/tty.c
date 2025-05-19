#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vm.h>
#include <kernel/pc_font.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern u8 _binary_font_psfu_start;
extern u8 _binary_font_psfu_end;

struct framebuffer display = {0};
const struct pc_font *font = (struct pc_font *)&_binary_font_psfu_start;
u8 *font_start;
size_t TERMINAL_WIDTH, TERMINAL_HEIGHT;
size_t row, column;
u16 fg, bg;
char *tty_buffer;

int terminal_initialize(struct framebuffer *fb) {
    memcpy(&display, fb, sizeof(struct framebuffer));
    // display.addr is still a physical address
    // asking to map it to a virtual one
    u64 vaddr =
        (u64)map_mmio(NULL, display.addr, display.pitch * display.height, true);
    if (vaddr == 0) {
        logf(ERROR, "MMIO mapping for terminal buffer failed");
        return 1;
    }
    display.addr = vaddr;

    font_start = (u8 *)(&_binary_font_psfu_start + font->headersize);

    TERMINAL_HEIGHT = display.height / (font->height + 1);
    TERMINAL_WIDTH = display.width / (font->width + 1);
    logf(INFO, "Init tty with size %dx%d", TERMINAL_WIDTH, TERMINAL_HEIGHT);
    row = 0;
    column = 0;

    fg = (u16)0xFFFFFF;
    bg = 0;

    tty_buffer = alloc(NULL, TERMINAL_HEIGHT * (TERMINAL_WIDTH + 1));

    return terminal_clear();
}

int terminal_clear() {
    if (display.addr == 0) {
        return 1;
    }

    memset(tty_buffer, 0, TERMINAL_HEIGHT * (TERMINAL_WIDTH + 1));
    memset((char *)display.addr, 0, display.pitch * display.height);
    return 0;
}

// TODO support other bpp than 32
void terminal_render_char(size_t x_org, size_t y_org, char c) {
    u8 *font_char = font_start + (c * font->bytesperglyph);

    u8 bitmap_offset;
    for (u32 y = 0; y < font->height; y++) {
        bitmap_offset = ceildiv(font->width, 8) * y;

        for (u32 x = 0; x < font->width; x += 8) {
            for (u8 i = 0; i < 8; i++) {
                if ((x + i) >= font->width) break;

                u32 *index = (u32 *)(display.addr + y_org + x_org +
                                     (x + i) * (display.bpp / 8));

                if (font_char[bitmap_offset] & (0x80 >> i))
                    *index = fg;
                else
                    *index = bg;
            }
            bitmap_offset++;
        }
        y_org += display.pitch;
    }
}

void terminal_render() {
    size_t x, y;
    for (y = 0; y < TERMINAL_HEIGHT; y++) {
        for (x = 0; x < TERMINAL_WIDTH; x++) {
            u64 y0 = (y * (font->height + 1) * display.pitch);
            u64 x0 = (x * (font->width + 1) * (display.bpp / 8));
            char c = tty_buffer[x + y * (TERMINAL_WIDTH + 1)];

            if (0x20 <= c && c < 0x7F) {
                // Printable character
                terminal_render_char(x0, y0, c);
                continue;
            }

            if (c == '\0' || c == '\n') {
                c = ' ';
            }

            terminal_render_char(x0, y0, c);
        }
    }
}

void terminal_putchar(u8 c) {
    if (column == TERMINAL_WIDTH) {
        terminal_return();
    }

    tty_buffer[column + row * (TERMINAL_WIDTH + 1)] = c;

    column++;
}

void terminal_move_up() {
    size_t y;
    for (y = 1; y < TERMINAL_HEIGHT; y++) {
        memcpy(&tty_buffer[(y - 1) * (TERMINAL_WIDTH + 1)],
               &tty_buffer[y * (TERMINAL_WIDTH + 1)], TERMINAL_WIDTH + 1);
    }

    memset(&tty_buffer[(y - 1) * (TERMINAL_WIDTH + 1)], 0, TERMINAL_WIDTH + 1);

    terminal_render();
}

void terminal_return() {
    tty_buffer[column + row * (TERMINAL_WIDTH + 1)] = '\n';
    if (++row == TERMINAL_HEIGHT) {
        terminal_move_up();
        row--;
    }
    column = 0;
}

void terminal_backspace() {
    if (column != 0) {
        column--;
        terminal_putchar('\0');
        column--;
    } else if (row != 0) {
        row--;
        column = 0;
        while (tty_buffer[column + row * (TERMINAL_WIDTH - 1)] != '\n')
            column++;
    }
}

void terminal_write(u8 data) {
    if (display.addr == 0) {
        return;
    }

    if (0x20 <= data && data < 0x7F) {
        terminal_putchar(data);
    }

    switch (data) {
        case '\b':
            terminal_backspace();
            tty_buffer[column + row * (TERMINAL_WIDTH + 1)] = '\0';
            break;
        case '\t':
            // TODO add support for \t
            break;
        case '\n':
            tty_buffer[column + row * (TERMINAL_WIDTH + 1)] = data;
            terminal_return();
            break;

        default:
            break;
    }

    terminal_render();
}

void terminal_writestring(const char *data) {
    int i = 0;
    while (data[i]) {
        terminal_write(data[i]);
        i++;
    }
}
