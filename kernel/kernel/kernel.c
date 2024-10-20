#include <stdio.h>
#include <stdlib.h>

#include <kernel/multiboot2.h>
#include <kernel/tty.h>

void main(unsigned long magic, unsigned long addr) {
    terminal_initialize();
    if (magic != 0x36d76289) {
        printf("multiboot2 magic is incorrect\n");
        abort();
    }

    if (addr & 7) {
        printf("Unaligned mbi: 0x%X\n", addr);
        abort();
    }

    unsigned int size = *(unsigned int *) addr;
    // printf("Announced size is 0x%X\n", size);
    struct multiboot_tag_header* header;
    for (header = (struct multiboot_tag_header *) (addr + 8);
         header->type != MULTIBOOT_HEADER_TAG_END;
         header = (struct multiboot_tag_header *) ((multiboot_u8_t *) header
                                                   + ((header->size + 7) & ~7))
    ) {
        switch(header->type) {
            case MULTIBOOT_TAG_TYPE_BOOT_CMD_LINE:
                printf("Boot command line=%s\n",
                       ((struct multiboot_boot_cmd_line *) header)->cmdline);
                break;
            case MULTIBOOT_TAG_TYPE_BOOTLOADER_NAME:
                printf("Boot loader name=%s\n",
                       ((struct multiboot_bootloader *) header)->name);
                break;
            case MULTIBOOT_TAG_TYPE_MODULES:
                struct multiboot_module *module = (struct multiboot_module *) header;
                printf("Module %s starts at %i end at %i\n",
                       module->string,
                       module->mod_start,
                       module->mod_end);
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                struct multiboot_basic_meminfo *meminfo = (struct multiboot_basic_meminfo *) header;
                printf("Meminfo:\n");
                printf("    lower: 0x%X\n", meminfo->mem_lower);
                printf("    upper: 0x%X\n", meminfo->mem_upper);
                break;
            case MULTIBOOT_TAG_TYPE_BIOS_BOOT_DEVICE:
                struct multiboot_bios_boot_device *bootdevice = (struct multiboot_bios_boot_device *) header;
                printf("Bootdevice:\n");
                printf("    biosdev:      0x%X\n", bootdevice->biosdev);
                printf("    partition:    0x%X\n", bootdevice->partition);
                printf("    subpartition: 0x%X\n", bootdevice->sub_partition);
                break;
            case MULTIBOOT_TAG_TYPE_MEMORY_MAP:
                struct multiboot_memory_map *mmap = (struct multiboot_memory_map *) header;
                int entry_number = (mmap->size - 16) / mmap->entry_size; // 16 is the size of the fixed part of the struct
                printf("Memory map:\n");
                printf("    entry_size: %d\n", mmap->entry_size);
                printf("    entry_version: %d\n", mmap->entry_version);
                printf("    entries_number: %d\n", (mmap->size - 16) / mmap->entry_size);
                struct multiboot_mmap_entry entry;
                for (int i = 0; i < entry_number; i ++) {
                    entry = mmap->entries[i];
                    // printf("    Entry %i:\n", i);
                    // printf("        base_addr: 0x%X%X\n", (unsigned) (entry.base_addr >> 32), (unsigned) (entry.base_addr & 0xffffffff));
                    // printf("        length: 0x%X%X\n", (unsigned) (entry.length >> 32), (unsigned) (entry.length & 0xffffffff));
                    // printf("        type: %d\n", entry.type);
                }
                break;
            case MULTIBOOT_TAG_TYPE_VBE_INFO:
                struct multiboot_vbe *vbe = (struct multiboot_vbe *) header;
                printf("VBE\n");
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER_INFO:
                struct multiboot_framebuffer *fb = (struct multiboot_framebuffer *) header;
                printf("Framebuffer\n");
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SYMBOLS:
                struct multiboot_elf_symbols *symbols = (struct multiboot_elf_symbols *) header;
                printf("ELF Symbols\n");
                break;
            case MULTIBOOT_TAG_TYPE_APM_TABLE:
                struct multiboot_apm *apm = (struct multiboot_apm *) header;
                printf("APM\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI32_TP:
                struct multiboot_efi32 *efi32 = (struct multiboot_efi32 *) header;
                printf("EFI32\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI64_TP:
                struct multiboot_efi64 *efi64 = (struct multiboot_efi64 *) header;
                printf("EFI64\n");
                break;
            case MULTIBOOT_TAG_TYPE_SMBIOS:
                struct multiboot_smbios *smbios = (struct multiboot_smbios *) header;
                printf("SMBios\n");
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                struct multiboot_acpi_old *acpi_old = (struct multiboot_acpi_old *) header;
                printf("ACPI old\n");
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                struct multiboot_acpi_new *acpi_new = (struct multiboot_acpi_new *) header;
                printf("ACPI new\n");
                break;
            case MULTIBOOT_TAG_TYPE_NETWORK:
                struct multiboot_network *network = (struct multiboot_network *) header;
                printf("Network\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI_MMAP:
                struct multiboot_efi_mmap *efi_mmap = (struct multiboot_efi_mmap *) header;
                printf("EFI Memory map\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI_BOOT_SERVICE:
                printf("Boot service still enabled\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI32_IMAGE_HANDLE:
                struct multiboot_efi32_im *efi32_im = (struct multiboot_efi32_im *) header;
                printf("EFI32 IH\n");
                break;
            case MULTIBOOT_TAG_TYPE_EFI64_IMAGE_HANDLE:
                struct multiboot_efi64_im *efi64_im = (struct multiboot_efi64_im *) header;
                printf("EFI64 IH\n");
                break;
            case MULTIBOOT_TAG_TYPE_BASE_ADDRESS:
                struct multiboot_load_base_addr *load_addr = (struct multiboot_load_base_addr *) header;
                printf("Load address: 0x%X\n", load_addr->addr);
                break;
            default:
                printf("Unrecognized tag of type=%d and size=%d\n", header->type, header->size);
                break;
        }
    }

}
