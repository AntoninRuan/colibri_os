#ifndef KERNEL_MULTIBOOT2_H
#define KERNEL_MULTIBOOT2_H

#define MULTIBOOT_MAGIC 0xE85250D6

#define MULTIBOOT_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT_ARCH_I386   0
#define MULTIBOOT_ARCH_MIPS32 4

#define MULTIBOOT_HEADER_TAG_END                 0
#define MULTIBOOT_HEADER_TAG_INFO_REQUEST        1
#define MULTIBOOT_HEADER_TAG_ADDRESS             2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS       3
#define MULTIBOOT_HEADER_TAG_FLAGS               4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER         5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGNMENT    6
#define MULTIBOOT_HEADER_TAG_EFI_BOOT_SERVICE    7
#define MULTIBOOT_HEADER_TAG_I386_ENTRY_ADDRESS  8
#define MULTIBOOT_HEADER_TAG_AMD64_ENTRY_ADDRESS 9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE         10

#define MULTIBOOT_HEADER_FLAG_OPTIONAL 1

#define MULTIBOOT_TAG_TYPE_BOOT_CMD_LINE      1
#define MULTIBOOT_TAG_TYPE_BOOTLOADER_NAME    2
#define MULTIBOOT_TAG_TYPE_MODULES            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO      4
#define MULTIBOOT_TAG_TYPE_BIOS_BOOT_DEVICE   5
#define MULTIBOOT_TAG_TYPE_MEMORY_MAP         6
#define MULTIBOOT_TAG_TYPE_VBE_INFO           7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER_INFO   8
#define MULTIBOOT_TAG_TYPE_ELF_SYMBOLS        9
#define MULTIBOOT_TAG_TYPE_APM_TABLE          10
#define MULTIBOOT_TAG_TYPE_EFI32_TP           11
#define MULTIBOOT_TAG_TYPE_EFI64_TP           12
#define MULTIBOOT_TAG_TYPE_SMBIOS             13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD           14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW           15
#define MULTIBOOT_TAG_TYPE_NETWORK            16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP           17
#define MULTIBOOT_TAG_TYPE_EFI_BOOT_SERVICE   18
#define MULTIBOOT_TAG_TYPE_EFI32_IMAGE_HANDLE 19
#define MULTIBOOT_TAG_TYPE_EFI64_IMAGE_HANDLE 20
#define MULTIBOOT_TAG_TYPE_BASE_ADDRESS       21

#ifndef __is_asm
#include <kernel/vga.h>
#include <stdint.h>

struct multiboot_tag_header {
    u32 type;
    u32 size;
};

struct multiboot_boot_cmd_line {
    u32 type;
    u32 size;
    char cmdline[0];
};

struct multiboot_bootloader {
    u32 type;
    u32 size;
    char name[0];
};

struct multiboot_module {
    u32 type;
    u32 size;
    u32 mod_start;
    u32 mod_end;
    char string[0];
};

struct multiboot_basic_meminfo {
    u32 type;
    u32 size;
    u32 mem_lower;
    u32 mem_upper;
};

struct multiboot_bios_boot_device {
    u32 type;
    u32 size;
    u32 biosdev;
    u32 partition;
    u32 sub_partition;
};

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2
#define MULTIBOOT_MEMORY_ACPI      3
#define MULTIBOOT_MEMORY_NVS       4
#define MULTIBOOT_MEMORY_BADRAM    5

struct multiboot_mmap_entry {
    u64 base_addr;
    u64 length;
    u32 type;
    u32 reserved;
};

struct multiboot_memory_map {
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
    struct multiboot_mmap_entry entries[0];
};

struct multiboot_vbe {
    u32 type;
    u32 size;
    u16 vbe_mode;
    u16 vbe_interface_segment;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    u8 vbe_control_info[512];
    u8 vbe_mode_info[256];
};

struct multiboot_framebuffer {
    u32 type;
    u32 size;
    struct framebuffer fb;
};

struct multiboot_elf_symbols {
    u32 type;
    u32 size;
    u16 num;
    u16 entsize;
    u16 shndx;
    u16 reserved;
    char sections[0];
};

struct multiboot_apm {
    u32 type;
    u32 size;
    u16 version;
    u16 cseg;
    u32 offset;
    u16 cseg16;
    u16 dseg;
    u16 flags;
    u16 cseg_len;
    u16 cseg16_len;
    u16 dseg_len;
};

struct multiboot_efi32 {
    u32 type;
    u32 size;
    u32 pointer;
};

struct multiboot_efi64 {
    u32 type;
    u32 size;
    u64 pointer;
};

struct multiboot_smbios {
    u32 type;
    u32 size;
    u8 major;
    u8 minor;
    u8 reserved[6];
    u8 tables[0];
};

struct multiboot_acpi_old {
    u32 type;
    u32 size;
    u8 rsdp[0];
};

struct multiboot_acpi_new {
    u32 type;
    u32 size;
    u8 rsdp[0];
};

struct multiboot_network {
    u32 type;
    u32 size;
    u8 dhcpack[0];
};

struct multiboot_efi_mmap {
    u32 type;
    u32 size;
    u32 descriptor_size;
    u32 descriptor_version;
    u8 mmap[0];
};

struct multiboot_efi32_im {
    u32 type;
    u32 size;
    u32 pointer;
};

struct multiboot_efi64_im {
    u32 type;
    u32 size;
    u64 pointer;
};

struct multiboot_load_base_addr {
    u32 type;
    u32 size;
    u32 addr;
};

struct multiboot_boot_information {
    struct multiboot_boot_cmd_line **cmd_line;
    struct multiboot_bootloader **bootloader;
    struct multiboot_module **module;
    u64 module_size;
    struct multiboot_basic_meminfo **basic_meminfo;
    struct multiboot_bios_boot_device **bios_boot_device;
    struct multiboot_memory_map **memory_map;
    struct multiboot_vbe **vbe;
    struct multiboot_framebuffer **framebuffer;
    struct multiboot_elf_symbols **elf_symbols;
    struct multiboot_apm **apm;
    struct multiboot_efi32 **efi32;
    struct multiboot_efi64 **efi64;
    struct multiboot_smbios **smbios;
    struct multiboot_acpi_old **acpi_old;
    struct multiboot_acpi_new **acpi_new;
    struct multiboot_network **network;
    struct multiboot_efi_mmap **efi_mmap;
    struct multiboot_efi32_im **efi32_handle;
    struct multiboot_efi64_im **efi64_handle;
    struct multiboot_load_base_addr **load_base_addr;
};

void load_multiboot_info(u32 magic, u64 addr,
                         struct multiboot_boot_information *);

#endif  // __is_asm

#endif  // KERNEL_MULTIBOOT2_H
