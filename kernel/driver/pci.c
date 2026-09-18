#include <kernel/driver/pci.h>
#include <kernel/list.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <stdint.h>
#include <string.h>

// Store pcie device as a linked list
pci_device_lst pci_devices = {0};

static inline void *pci_addr(void *base, u8 bus, u8 device, u8 function, u8 offset) {
    return (void *)(base + ((u64)bus << 20) + ((u64)device << 15)
                    + ((u64)function << 12) + ((u64)offset));
}

void scan_pcie_bus_group(uintptr_t phys_addr, u32 bus_count) {
    void *base_addr =
        map_mmio(NULL, phys_addr, bus_count * (1 << 20), true, true);

    if (!base_addr) return;

    for (u32 bus = 0; bus < bus_count; bus++) {
        for (u32 device_nbr = 0; device_nbr < 32; device_nbr++) {
            pci_common_header *header =
                pci_addr(base_addr, bus, device_nbr, 0, 0);
            if ((header->vendor_id & 0xFFFF) != 0xFFFF) {
                u8 function_count = 1;
                if (header->header_type & 0x80)
                    // Multifunction device
                    function_count = 8;

                // Check all the functions of the device
                for (u8 f = 0; f < function_count; f++) {
                    header = pci_addr(base_addr, bus, device_nbr, f, 0);

                    // Single function device
                    if ((header->vendor_id & 0xFFFF) == 0xFFFF) continue;

                    pci_device *device = alloc(sizeof(pci_device));
                    memset(device, 0, sizeof(pci_device));
                    device->bus_nbr = bus;
                    device->device_nbr = device_nbr;
                    device->function_nbr = f;

                    memcpy(&device->header, header, sizeof(pci_common_header));

                    device->phys_addr =
                        (uintptr_t)(phys_addr + ((uintptr_t)bus << 20)
                                    + ((uintptr_t)device_nbr << 15));
                    lst_init(&device->capabilities);

                    logf(INFO, "PCI device detected at %d:%d.%d: %s",
                         (u32)device->bus_nbr, (u32)device->device_nbr,
                         (u32)device->function_nbr,
                         pci_class_name[device->header.class]
                                       [device->header.subclass]);
                    lst_push(&pci_devices, device);
                }
            }
        }
    }

    unmap_mmio(base_addr);
}

void register_pcie_device(acpi_mcfg_table *mcfg_table) {
    pci_config_space_alloc cur_config_space = mcfg_table->config_spaces[0];
    uintptr_t phys_addr = cur_config_space.base_addr;
    u32 bus_count = cur_config_space.end_pci_bus_number
                    - cur_config_space.start_pci_bus_number + 1;

    scan_pcie_bus_group(phys_addr, bus_count);
}

void init_pci() {
    acpi_mcfg_table *table = (acpi_mcfg_table *)find_table("MCFG");
    lst_init(&pci_devices);
    register_pcie_device(table);
}

bool pci_is_multifunction(pci_device *dev) {
    if (!dev) return false;

    return (bool)(dev->header.header_type & 0x80);
}

void *pci_map_device(pci_device *dev) {
    if (dev->mapped_addr)
        // Device is already mapped
        return dev->mapped_addr;

    size_t config_space_size = 4096;
    if (pci_is_multifunction(dev)) config_space_size *= 8;

    dev->mapped_addr =
        map_mmio(NULL, dev->phys_addr, config_space_size, true, false);
    return dev->mapped_addr;
}

void pci_unmap_device(pci_device *dev) {
    if (!dev->mapped_addr)
        // Device is not mapped
        return;

    unmap_mmio(dev->mapped_addr);
}

pci_cap *pci_find_cap(pci_device *dev, u16 cap_id, u16 cap_type) {
    if (lst_empty(&dev->capabilities)) return NULL;

    pci_cap *cur = (pci_cap *)dev->capabilities.next;
    for (; cur != (void *)&dev->capabilities; cur = cur->next) {
        if (cur->id == cap_id && cur->type == cap_type) return cur;
    }

    return NULL;
}

void pci_scan_cap_base(pci_device *dev) {
    if (!dev->header.status.capabilities_list)
        // No cap list
        return;

    bool was_mapped = dev->mapped_addr != 0;
    if (!was_mapped)
        if (!pci_map_device(dev))
            // Error while mapping device
            return;

    pci_header_0 *header = (pci_header_0 *)dev->mapped_addr;

    u8 offset = header->cap_pointer & (u8)~3;

    while (offset) {
        u8 id = *(u8 *)(dev->mapped_addr + offset);
        u8 next = *(u8 *)(dev->mapped_addr + offset + 1);
        pci_cap *cap = alloc(sizeof(pci_cap));
        if (id) {
            cap->id = (u16)id;
            cap->offset = (u16)offset;
            cap->type = PCI_CAP_TYPE_BASE;
            lst_push(&dev->capabilities, cap);
        }
        offset = next & (u8)~3;
    }

    if (!was_mapped) pci_unmap_device(dev);
}

void pci_scan_cap_ext(pci_device *dev) {
    if (!pci_find_cap(dev, PCI_CAP_BASE_PCIE, PCI_CAP_TYPE_BASE)) return;



    // TODO implement fun
    return;
}

void pci_scan_cap(pci_device *dev, u8 wanted) {
    if (wanted & PCI_CAP_WANTED_EXT) wanted |= PCI_CAP_WANTED_BASE;

    if (wanted & PCI_CAP_WANTED_BASE) pci_scan_cap_base(dev);

    if (wanted & PCI_CAP_WANTED_EXT) pci_scan_cap_ext(dev);
}
