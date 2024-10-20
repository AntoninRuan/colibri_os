#ifndef MULTIBOOT2_H
#define MULTIBOOT2_H

#define MULTIBOOT_MAGIC 0xE85250D6

#define MULTIBOOT_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT_ARCH_I386   0
#define MULTIBOOT_ARCH_MIPS32 4

#define MULTIBOOT_HEADER_TAG_END                  0
#define MULTIBOOT_HEADER_TAG_INFO_REQUEST         1
#define MULTIBOOT_HEADER_TAG_ADDRESS              2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS        3
#define MULTIBOOT_HEADER_TAG_FLAGS                4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER          5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGNMENT     6
#define MULTIBOOT_HEADER_TAG_EFI_BOOT_SERVICE     7
#define MULTIBOOT_HEADER_TAG_I386_ENTRY_ADDRESS   8
#define MULTIBOOT_HEADER_TAG_AMD64_ENTRY_ADDRESS  9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE          10
#define MULTIBOOT_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT_TAG_TYPE_BOOT_CMD_LINE          1
#define MULTIBOOT_TAG_TYPE_BOOTLOADER_NAME        2
#define MULTIBOOT_TAG_TYPE_MODULES                3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO          4
#define MULTIBOOT_TAG_TYPE_BIOS_BOOT_DEVICE       5
#define MULTIBOOT_TAG_TYPE_MEMORY_MAP             6
#define MULTIBOOT_TAG_TYPE_VBE_INFO               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER_INFO       8
#define MULTIBOOT_TAG_TYPE_ELF_SYMBOLS            9
#define MULTIBOOT_TAG_TYPE_APM_TABLE              10
#define MULTIBOOT_TAG_TYPE_EFI32_TP               11
#define MULTIBOOT_TAG_TYPE_EFI64_TP               12
#define MULTIBOOT_TAG_TYPE_SMBIOS                 13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD               14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW               15
#define MULTIBOOT_TAG_TYPE_NETWORK                16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP               17
#define MULTIBOOT_TAG_TYPE_EFI_BOOT_SERVICE       18
#define MULTIBOOT_TAG_TYPE_EFI32_IMAGE_HANDLE     19
#define MULTIBOOT_TAG_TYPE_EFI64_IMAGE_HANDLE     20
#define MULTIBOOT_TAG_TYPE_BASE_ADDRESS           21

#ifndef __is_asm
typedef unsigned char         multiboot_u8_t;
typedef unsigned short        multiboot_u16_t;
typedef unsigned int          multiboot_u32_t;
typedef unsigned long long    multiboot_u64_t;

struct multiboot_tag_header {
    multiboot_u32_t type;
    multiboot_u32_t size;
};

struct multiboot_boot_cmd_line {
    multiboot_u32_t type;
    multiboot_u32_t size;
    char cmdline[0];
};

struct multiboot_bootloader {
    multiboot_u32_t type;
    multiboot_u32_t size;
    char name[0];
};

struct multiboot_module {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t mod_start;
    multiboot_u32_t mod_end;
    char string[0];
};

struct multiboot_basic_meminfo {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t mem_lower;
    multiboot_u32_t mem_upper;
};

struct multiboot_bios_boot_device {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t biosdev;
    multiboot_u32_t partition;
    multiboot_u32_t sub_partition;
};

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2
#define MULTIBOOT_MEMORY_ACPI      3
#define MULTIBOOT_MEMORY_NVS       4
#define MULTIBOOT_MEMORY_BADRAM    5

struct multiboot_mmap_entry {
    multiboot_u64_t base_addr;
    multiboot_u64_t length;
    multiboot_u32_t type;
    multiboot_u32_t reserved;
};

struct multiboot_memory_map {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t entry_size;
    multiboot_u32_t entry_version;
    struct multiboot_mmap_entry entries[0];
};

struct multiboot_vbe {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u16_t vbe_mode;
    multiboot_u16_t vbe_interface_segment;
    multiboot_u16_t vbe_interface_off;
    multiboot_u16_t vbe_interface_len;
    multiboot_u8_t vbe_control_info[512];
    multiboot_u8_t vbe_mode_info[256];
};

struct multiboot_color {
    multiboot_u8_t red;
    multiboot_u8_t green;
    multiboot_u8_t blue;
};

struct multiboot_framebuffer {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u64_t addr;
    multiboot_u32_t pitch;
    multiboot_u32_t width;
    multiboot_u32_t height;
    multiboot_u8_t bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA     2
    multiboot_u8_t fb_type;
    multiboot_u8_t reserved;

    union {
        struct
        {
            multiboot_u16_t palette_num_colors;
            struct multiboot_color palette[0];
        };
        struct {
            multiboot_u8_t red_field_position;
            multiboot_u8_t red_mask_size;
            multiboot_u8_t green_field_position;
            multiboot_u8_t green_mask_size;
            multiboot_u8_t blue_field_position;
            multiboot_u8_t blue_mask_size;
        };
    }
};

struct multiboot_elf_symbols {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u16_t num;
    multiboot_u16_t entsize;
    multiboot_u16_t shndx;
    multiboot_u16_t reserved;
    char sections[0];
};

struct multiboot_apm {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u16_t version;
    multiboot_u16_t cseg;
    multiboot_u32_t offset;
    multiboot_u16_t cseg16;
    multiboot_u16_t dseg;
    multiboot_u16_t flags;
    multiboot_u16_t cseg_len;
    multiboot_u16_t cseg16_len;
    multiboot_u16_t dseg_len;
};

struct multiboot_efi32 {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t pointer;
};

struct multiboot_efi64 {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u64_t pointer;
};

struct multiboot_smbios {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u8_t major;
    multiboot_u8_t minor;
    multiboot_u8_t reserved[6];
    multiboot_u8_t tables[0];
};

struct multiboot_acpi_old {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u8_t rsdp[0];
};

struct multiboot_acpi_new {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u8_t rsdp[0];
};

struct multiboot_network {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u8_t dhcpack[0];
};

struct multiboot_efi_mmap {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t descriptor_size;
    multiboot_u32_t descriptor_version;
    multiboot_u8_t mmap[0];
};

struct multiboot_efi32_im {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t pointer;
};

struct multiboot_efi64_im {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u64_t pointer;
};

struct multiboot_load_base_addr {
    multiboot_u32_t type;
    multiboot_u32_t size;
    multiboot_u32_t addr;
};

#endif // __is_asm

#endif // MULTIBOOT2_H
