#ifndef KERNEL_MULTIBOOT2_H
#define KERNEL_MULTIBOOT2_H

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

#define MULTIBOOT_HEADER_FLAG_OPTIONAL            1

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
#include <stdint.h>
#include <kernel/vga.h>

struct multiboot_tag_header {
    uint32_t type;
    uint32_t size;
};

struct multiboot_boot_cmd_line {
    uint32_t type;
    uint32_t size;
    char cmdline[0];
};

struct multiboot_bootloader {
    uint32_t type;
    uint32_t size;
    char name[0];
};

struct multiboot_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char string[0];
};

struct multiboot_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot_bios_boot_device {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t partition;
    uint32_t sub_partition;
};

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED  2
#define MULTIBOOT_MEMORY_ACPI      3
#define MULTIBOOT_MEMORY_NVS       4
#define MULTIBOOT_MEMORY_BADRAM    5

struct multiboot_mmap_entry {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct multiboot_memory_map {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[0];
};

struct multiboot_vbe {
    uint32_t type;
    uint32_t size;
    uint16_t vbe_mode;
    uint16_t vbe_interface_segment;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint8_t vbe_control_info[512];
    uint8_t vbe_mode_info[256];
};

struct multiboot_framebuffer {
    uint32_t type;
    uint32_t size;
    struct framebuffer fb;
};

struct multiboot_elf_symbols {
    uint32_t type;
    uint32_t size;
    uint16_t num;
    uint16_t entsize;
    uint16_t shndx;
    uint16_t reserved;
    char sections[0];
};

struct multiboot_apm {
    uint32_t type;
    uint32_t size;
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t cseg_len;
    uint16_t cseg16_len;
    uint16_t dseg_len;
};

struct multiboot_efi32 {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot_efi64 {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot_smbios {
    uint32_t type;
    uint32_t size;
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t tables[0];
};

struct multiboot_acpi_old {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[0];
};

struct multiboot_acpi_new {
    uint32_t type;
    uint32_t size;
    uint8_t rsdp[0];
};

struct multiboot_network {
    uint32_t type;
    uint32_t size;
    uint8_t dhcpack[0];
};

struct multiboot_efi_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t descriptor_size;
    uint32_t descriptor_version;
    uint8_t mmap[0];
};

struct multiboot_efi32_im {
    uint32_t type;
    uint32_t size;
    uint32_t pointer;
};

struct multiboot_efi64_im {
    uint32_t type;
    uint32_t size;
    uint64_t pointer;
};

struct multiboot_load_base_addr {
    uint32_t type;
    uint32_t size;
    uint32_t addr;
};

void load_multiboot_info(uint32_t magic, uint32_t addr);

#endif // __is_asm

#endif // KERNEL_MULTIBOOT2_H
