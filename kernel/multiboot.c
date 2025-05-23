#include <kernel/kernel.h>
#include <kernel/log.h>
#include <kernel/multiboot2.h>
#include <kernel/tty.h>
#include <stddef.h>
#include <stdint.h>

void load_multiboot_info(u32 magic, u64 addr,
                         struct multiboot_boot_information *boot_info) {
    log(INFO, "Loading multiboot info");
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("load_multiboot_info: multiboot magic does not correspond");

    // if (addr & 7) {
    //     abort();
    // }

    // TODO probably should have verification we don't overflow size
    // unsigned int size = *(unsigned int *) addr;
    struct multiboot_tag_header *header;
    u32 module_index = 0;
    for (header = (struct multiboot_tag_header *)(addr + 8);
         header->type != MULTIBOOT_HEADER_TAG_END;
         header = (struct multiboot_tag_header *)((u8 *)header +
                                                  ((header->size + 7) & ~7))) {
        switch (header->type) {
            case MULTIBOOT_TAG_TYPE_BOOT_CMD_LINE:
                break;
            case MULTIBOOT_TAG_TYPE_BOOTLOADER_NAME:
                break;
            case MULTIBOOT_TAG_TYPE_MODULES:
                if (boot_info->module &&
                    module_index < boot_info->module_size) {
                    boot_info->module[module_index] =
                        (struct multiboot_module *)header;
                    module_index++;
                }
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                if (boot_info->basic_meminfo)
                    *boot_info->basic_meminfo =
                        (struct multiboot_basic_meminfo *)header;
                break;

            case MULTIBOOT_TAG_TYPE_BIOS_BOOT_DEVICE:
                if (boot_info->bios_boot_device)
                    *boot_info->bios_boot_device =
                        (struct multiboot_bios_boot_device *)header;
                break;

            case MULTIBOOT_TAG_TYPE_MEMORY_MAP:
                if (boot_info->memory_map)
                    *boot_info->memory_map =
                        (struct multiboot_memory_map *)header;
                break;

            case MULTIBOOT_TAG_TYPE_VBE_INFO:
                if (boot_info->vbe)
                    boot_info->vbe = (struct multiboot_vbe **)&header;
                break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER_INFO:
                if (boot_info->framebuffer)
                    *boot_info->framebuffer =
                        (struct multiboot_framebuffer *)header;
                break;

            case MULTIBOOT_TAG_TYPE_ELF_SYMBOLS:
                if (boot_info->elf_symbols)
                    *boot_info->elf_symbols =
                        (struct multiboot_elf_symbols *)header;
                break;

            case MULTIBOOT_TAG_TYPE_APM_TABLE:
                if (boot_info->apm)
                    *boot_info->apm = (struct multiboot_apm *)header;
                break;

            case MULTIBOOT_TAG_TYPE_EFI32_TP:
                if (boot_info->efi32)
                    *boot_info->efi32 = (struct multiboot_efi32 *)header;
                break;

            case MULTIBOOT_TAG_TYPE_EFI64_TP:
                if (boot_info->efi64)
                    *boot_info->efi64 = (struct multiboot_efi64 *)header;
                break;

            case MULTIBOOT_TAG_TYPE_SMBIOS:
                if (boot_info->smbios)
                    *boot_info->smbios = (struct multiboot_smbios *)header;
                break;

            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                if (boot_info->acpi_old)
                    *boot_info->acpi_old = (struct multiboot_acpi_old *)header;
                break;

            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                if (boot_info->acpi_new)
                    *boot_info->acpi_new = (struct multiboot_acpi_new *)header;
                break;

            case MULTIBOOT_TAG_TYPE_NETWORK:
                if (boot_info->network)
                    *boot_info->network = (struct multiboot_network *)header;
                break;

            case MULTIBOOT_TAG_TYPE_EFI_MMAP:
                if (boot_info->efi_mmap)
                    *boot_info->efi_mmap = (struct multiboot_efi_mmap *)header;
                break;

            case MULTIBOOT_TAG_TYPE_EFI_BOOT_SERVICE:
                break;

            case MULTIBOOT_TAG_TYPE_EFI32_IMAGE_HANDLE:
                if (boot_info->efi32_handle)
                    *boot_info->efi32_handle =
                        (struct multiboot_efi32_im *)header;
                break;

            case MULTIBOOT_TAG_TYPE_EFI64_IMAGE_HANDLE:
                if (boot_info->efi64_handle)
                    *boot_info->efi64_handle =
                        (struct multiboot_efi64_im *)header;
                break;

            case MULTIBOOT_TAG_TYPE_BASE_ADDRESS:
                if (boot_info->load_base_addr)
                    *boot_info->load_base_addr =
                        (struct multiboot_load_base_addr *)header;
                break;

            default:
                break;
        }
    }

    boot_info->module_size = module_index;

    return;
}
