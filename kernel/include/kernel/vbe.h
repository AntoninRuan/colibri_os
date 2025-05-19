/*
http://www.petesqbsite.com/sections/tutorials/tuts/vbe3.pdf
*/

#ifndef KERNEL_VBE_H
#define KERNEL_VBE_H

#ifndef __is_asm
#include <stdint.h>

struct vbe_info {
    char signature[4];  // Should always be "VESA"
    u16 version;
    u32 oem_string_ptr;
    u8 capabalities[4];
    u32 video_mode_ptr;
    u16 total_memory;
    u16 oem_software_rev;
    u32 oem_vendor_name_ptr;
    u32 oem_product_name_ptr;
    u32 oem_product_rev_ptr;
    u8 reserved[222];
    u8 oem_data[256];
};

struct vbe_mode_info {
    // Mandatory for all VBE
    u16 mode_attributes;
    u8 win_a_attributes;
    u8 win_b_attributes;
    u16 win_granularity;
    u16 win_size;
    u16 win_a_segment;
    u16 win_b_segment;
    u32 win_func_ptr;
    u16 pitch;  // bytes per horizontal line

    // Mandatory for VBE >= 1.2
    u16 width;
    u16 height;
    u8 x_char_size;
    u8 y_char_size;
    u8 planes;
    u8 bpp;
    u8 bank_number;
    u8 memory_model;
    u8 bank_size;  // in KB
    u8 image_number;
    u8 reserved_page;

    // Direct color fields (req for direct6 and YUV7 memory model)
    u8 red_mask_size;
    u8 red_field_position;
    u8 green_mask_size;
    u8 green_field_position;
    u8 blue_mask_size;
    u8 blue_field_position;
    u8 rsvd_mask_size;
    u8 rsvd_field_position;
    u8 direct_color_mode;

    // Mandatory for VBE >= 2.0
    u32 base_addr;
    u32 off_screen_mem_off;
    u16 off_screen_mem_size;

    // Mandatory for VBE >= 3.0
    u16 linear_pitch;
    u8 bank_image_number;
    u8 linear_image_number;
    u8 linear_red_mask_size;
    u8 linear_red_field_position;
    u8 linear_green_mask_size;
    u8 linear_green_field_position;
    u8 linear_blue_mask_size;
    u8 linear_blue_field_position;
    u8 linear_rsvd_mask_size;
    u8 linear_rsvd_field_position;
    u32 max_pixel_clock;

    u8 reserved[189];
};

#endif  // __is_asm

#endif  // KERNEL_VBE_H
