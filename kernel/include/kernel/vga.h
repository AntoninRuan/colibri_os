#ifndef VGA_H
#define VGA_H

#include <sys/cdefs.h>

struct color {
    u8 red;
    u8 green;
    u8 blue;
};

struct framebuffer {
    void *addr;
    u32 pitch;  // bytes per horizontal line
    u32 width;
    u32 height;
    u8 bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA     2
    u8 fb_type;
    u8 reserved;

    union {
        // If fb_type == 0
        struct {
            u16 palette_num_colors;
            struct color palette[0];
        };
        // If fb_type == 1
        struct {
            u8 red_field_position;
            u8 red_mask_size;
            u8 green_field_position;
            u8 green_mask_size;
            u8 blue_field_position;
            u8 blue_mask_size;
        };
    };
};

#endif  // VGA_H
