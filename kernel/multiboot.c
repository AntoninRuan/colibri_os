#include <stdlib.h>

#include <kernel/acpi.h>
#include <kernel/multiboot2.h>
#include <kernel/tty.h>

void load_multiboot_info(uint32_t magic, uint32_t addr) {
    if (magic != 0x36d76289) {
        abort();
    }

    if (addr & 7) {
        abort();
    }

    // unsigned int size = *(unsigned int *) addr;
    struct multiboot_tag_header* header;
    for (header = (struct multiboot_tag_header *) (addr + 8);
         header->type != MULTIBOOT_HEADER_TAG_END;
         header = (struct multiboot_tag_header *) ((uint8_t *) header
                                                   + ((header->size + 7) & ~7))
    ) {
        switch(header->type) {
            case MULTIBOOT_TAG_TYPE_BOOT_CMD_LINE:
                break;
            case MULTIBOOT_TAG_TYPE_BOOTLOADER_NAME:
                break;
            case MULTIBOOT_TAG_TYPE_MODULES:
                struct multiboot_module *module = (struct multiboot_module *) header;
                (void) module;
                break;
            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                struct multiboot_basic_meminfo *meminfo = (struct multiboot_basic_meminfo *) header;
                (void) meminfo;
                break;
            case MULTIBOOT_TAG_TYPE_BIOS_BOOT_DEVICE:
                struct multiboot_bios_boot_device *bootdevice = (struct multiboot_bios_boot_device *) header;
                (void) bootdevice;
                break;
            case MULTIBOOT_TAG_TYPE_MEMORY_MAP:
                struct multiboot_memory_map *mmap = (struct multiboot_memory_map *) header;
                int entry_number = (mmap->size - 16) / mmap->entry_size; // 16 is the size of the fixed part of the struct
                struct multiboot_mmap_entry entry;
                for (int i = 0; i < entry_number; i ++) {
                    entry = mmap->entries[i];
                    (void) entry;
                }
                break;
            case MULTIBOOT_TAG_TYPE_VBE_INFO:
                struct multiboot_vbe *vbe = (struct multiboot_vbe *) header;
                struct vbe_mode_info *vbe_mode_info = (struct vbe_mode_info *) &(vbe->vbe_mode_info);
                struct vbe_info *vbe_info = (struct vbe_info *) &(vbe->vbe_control_info);
                (void) vbe_mode_info;
                (void) vbe_info;
                break;
            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER_INFO:
                struct multiboot_framebuffer *fb = (struct multiboot_framebuffer *) header;
                // printf("Framebuffer:\n");
                // printf("    addr: 0x%X%X\n",
                //        (unsigned) (fb->addr >> 32),
                //        (unsigned) (fb->addr & 0xffffffff));
                // printf("    width: %d\n", fb->width);
                // printf("    height: %d\n", fb->height);
                // printf("    type: %d\n", fb->fb_type);
                terminal_initialize(&fb->fb);
                break;
            case MULTIBOOT_TAG_TYPE_ELF_SYMBOLS:
                struct multiboot_elf_symbols *symbols = (struct multiboot_elf_symbols *) header;
                (void) symbols;
                break;
            case MULTIBOOT_TAG_TYPE_APM_TABLE:
                struct multiboot_apm *apm = (struct multiboot_apm *) header;
                (void) apm;
                break;
            case MULTIBOOT_TAG_TYPE_EFI32_TP:
                struct multiboot_efi32 *efi32 = (struct multiboot_efi32 *) header;
                (void) efi32;
                break;
            case MULTIBOOT_TAG_TYPE_EFI64_TP:
                struct multiboot_efi64 *efi64 = (struct multiboot_efi64 *) header;
                (void) efi64;
                break;
            case MULTIBOOT_TAG_TYPE_SMBIOS:
                struct multiboot_smbios *smbios = (struct multiboot_smbios *) header;
                (void) smbios;
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
                struct multiboot_acpi_old *acpi_old = (struct multiboot_acpi_old *) header;
                struct rsdp *rsdp = (struct rsdp *) &acpi_old->rsdp;
                load_rsdp(rsdp);
                break;
            case MULTIBOOT_TAG_TYPE_ACPI_NEW:
                struct multiboot_acpi_new *acpi_new = (struct multiboot_acpi_new *) header;
                struct xsdp *xsdp = (struct xsdp *) &acpi_new->rsdp;
                load_xsdp(xsdp);
                break;
            case MULTIBOOT_TAG_TYPE_NETWORK:
                struct multiboot_network *network = (struct multiboot_network *) header;
                (void) network;
                break;
            case MULTIBOOT_TAG_TYPE_EFI_MMAP:
                struct multiboot_efi_mmap *efi_mmap = (struct multiboot_efi_mmap *) header;
                (void) efi_mmap;
                break;
            case MULTIBOOT_TAG_TYPE_EFI_BOOT_SERVICE:
                break;
            case MULTIBOOT_TAG_TYPE_EFI32_IMAGE_HANDLE:
                struct multiboot_efi32_im *efi32_im = (struct multiboot_efi32_im *) header;
                (void) efi32_im;
                break;
            case MULTIBOOT_TAG_TYPE_EFI64_IMAGE_HANDLE:
                struct multiboot_efi64_im *efi64_im = (struct multiboot_efi64_im *) header;
                (void) efi64_im;
                break;
            case MULTIBOOT_TAG_TYPE_BASE_ADDRESS:
                struct multiboot_load_base_addr *load_addr = (struct multiboot_load_base_addr *) header;
                (void) load_addr;
                break;
            default:
                break;
        }
    }

    return;
}
