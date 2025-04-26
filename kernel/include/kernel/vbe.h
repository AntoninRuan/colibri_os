/*
http://www.petesqbsite.com/sections/tutorials/tuts/vbe3.pdf
*/

#ifndef KERNEL_VBE_H
#define KERNEL_VBE_H

#ifndef __is_asm
#include <stdint.h>

struct vbe_info {
    char signature[4];  // Should always be "VESA"
    uint16_t version;
    uint32_t oem_string_ptr;
    uint8_t capabalities[4];
    uint32_t video_mode_ptr;
    uint16_t total_memory;
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    uint8_t reserved[222];
    uint8_t oem_data[256];
};

struct vbe_mode_info {
    // Mandatory for all VBE
    uint16_t mode_attributes;
    uint8_t win_a_attributes;
    uint8_t win_b_attributes;
    uint16_t win_granularity;
    uint16_t win_size;
    uint16_t win_a_segment;
    uint16_t win_b_segment;
    uint32_t win_func_ptr;
    uint16_t pitch;  // bytes per horizontal line

    // Mandatory for VBE >= 1.2
    uint16_t width;
    uint16_t height;
    uint8_t x_char_size;
    uint8_t y_char_size;
    uint8_t planes;
    uint8_t bpp;
    uint8_t bank_number;
    uint8_t memory_model;
    uint8_t bank_size;  // in KB
    uint8_t image_number;
    uint8_t reserved_page;

    // Direct color fields (req for direct6 and YUV7 memory model)
    uint8_t red_mask_size;
    uint8_t red_field_position;
    uint8_t green_mask_size;
    uint8_t green_field_position;
    uint8_t blue_mask_size;
    uint8_t blue_field_position;
    uint8_t rsvd_mask_size;
    uint8_t rsvd_field_position;
    uint8_t direct_color_mode;

    // Mandatory for VBE >= 2.0
    uint32_t base_addr;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;

    // Mandatory for VBE >= 3.0
    uint16_t linear_pitch;
    uint8_t bank_image_number;
    uint8_t linear_image_number;
    uint8_t linear_red_mask_size;
    uint8_t linear_red_field_position;
    uint8_t linear_green_mask_size;
    uint8_t linear_green_field_position;
    uint8_t linear_blue_mask_size;
    uint8_t linear_blue_field_position;
    uint8_t linear_rsvd_mask_size;
    uint8_t linear_rsvd_field_position;
    uint32_t max_pixel_clock;

    uint8_t reserved[189];
};

#endif  // __is_asm

#endif  // KERNEL_VBE_H
