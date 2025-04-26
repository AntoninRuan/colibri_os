#ifndef VGA_H
#define VGA_H

#include <stdint.h>

struct color {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct framebuffer {
    uint64_t addr;
    uint32_t pitch;  // bytes per horizontal line
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA     2
    uint8_t fb_type;
    uint8_t reserved;

    union {
        // If fb_type == 0
        struct {
            uint16_t palette_num_colors;
            struct color palette[0];
        };
        // If fb_type == 1
        struct {
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
        };
    };
};

#endif  // VGA_H
