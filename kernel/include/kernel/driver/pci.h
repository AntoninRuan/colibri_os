#ifndef PCIE_H
#define PCIE_H

#include <kernel/acpi.h>
#include <kernel/list.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define PCI_CLASS_UNCLASSIFIED            0x0
#define PCI_CLASS_MASS_STORAGE_CONTROLLER 0x1
#define PCI_CLASS_NETWORK_CONTROLLER      0x2
#define PCI_CLASS_DISPLAY_CONTROLLER      0x3
#define PCI_CLASS_MULTIMEDIA_CONTROLLER   0x4
#define PCI_CLASS_MEMORY_CONTROLLER       0x5
#define PCI_CLASS_BRIDGE                  0x6
#define PCI_CLASS_SIMPLE_COM_CONTROLLER   0x7
#define PCI_CLASS_BASE_SYS_PERIPHERAL     0x8
#define PCI_CLASS_INPUT_DEVICE_CONTROLLER 0x9

typedef struct pci_config_space_alloc pci_config_space_alloc;

struct pci_config_space_alloc {
    u64 base_addr;
    u16 segment_group_number;
    u8 start_pci_bus_number;
    u8 end_pci_bus_number;
    u32 reserved;
} __attribute__((packed));

typedef struct acpi_mcfg_table acpi_mcfg_table;

struct acpi_mcfg_table {
    acpi_sdt_header_t header;
    u64 reserved;
    pci_config_space_alloc config_spaces[];
} __attribute__((packed));

// RO: Read Only
// RW: Read Write
// RWC: Read/Write '1' to clear
// RW1C: Read/Write '1' to set

typedef union pci_cmd_reg pci_cmd_reg;

union pci_cmd_reg {
    struct {
        u16 io_space                 : 1;  // RW
        u16 mem_space                : 1;  // RW
        u16 bus_master               : 1;  // RW
        u16 special_cycle            : 1;  // RO
        u16 mem_write_and_invalidate : 1;  // RO
        u16 vga_palette_snoop        : 1;  // RO
        u16 parity_error             : 1;  // RW
        u16                          : 1;
        u16 serr_enable              : 1;  // RW
        u16 fast_b2b_enable          : 1;  // RO
        u16 interrupt_disable        : 1;  // RW
        u16                          : 5;
    } __attribute__((packed));
    u16 raw;
};

typedef union pci_status_reg pci_status_reg;

union pci_status_reg {
    struct {
        u16                          : 3;
        u16 int_status               : 1;  // RO
        u16 capabilities_list        : 1;  // RO
        u16 high_frequency           : 1;  // RO
        u16                          : 1;
        u16 fast_b2b_capable         : 1;  // RO
        u16 master_data_parity_error : 1;  // RW1C
        u16 devsel_timing            : 2;  // RO
        u16 sig_target_abort         : 1;  // RW1C
        u16 rec_target_abort         : 1;  // RW1C
        u16 rec_master_abort         : 1;  // RW1C
        u16 sig_master_abort         : 1;  // RW1C
        u16 detected_parity_error    : 1;  // RW1C
    } __attribute__((packed));
    u16 raw;
};

typedef struct pci_common_header pci_common_header;

struct pci_common_header {
    u16 vendor_id;
    u16 device_id;
    pci_cmd_reg command;
    pci_status_reg status;
    u8 rev_id;
    u8 prog_if;
    u8 subclass;
    u8 class;
    u8 cache_line_size;
    u8 latency_timer;
    u8 header_type;
    u8 bist;
};

typedef struct pci_header_0 pci_header_0;

struct pci_header_0 {
    pci_common_header common;
    u32 bar0;
    u32 bar1;
    u32 bar2;
    u32 bar3;
    u32 bar4;
    u32 bar5;
    u32 carbus_cis_pointer;
    u16 subsystem_vendor_id;
    u16 subsystem_id;
    u32 expansion_rom_bar;
    u8 cap_pointer;
    u8 reserved_0;
    u16 reserved_1;
    u32 reserved_2;
    u8 int_line;
    u8 int_pin;
    u8 min_grant;
    u8 max_latency;
};

typedef struct pci_cap pci_cap;
typedef struct lst pci_cap_lst;

struct pci_cap {
    pci_cap *prev;
    pci_cap *next;
    u16 id;
    u8 type;
    u16 offset;
};

#define PCI_CAP_TYPE_BASE 0x1
#define PCI_CAP_TYPE_EXT  0x2

#define PCI_CAP_BASE_ID_NULL              0x0
#define PCI_CAP_BASE_PWR_MGMT             0x1
#define PCI_CAP_BASE_AGP                  0x2
#define PCI_CAP_BASE_VPD                  0x3
#define PCI_CAP_BASE_SLOT_IDENT           0x4
#define PCI_CAP_BASE_MSI                  0x5
#define PCI_CAP_BASE_COMPACT_PCI_HOTSWAP  0x6
#define PCI_CAP_BASE_PCIX                 0x7
#define PCI_CAP_BASE_HYPERTRANSPORT       0x8
#define PCI_CAP_BASE_DEBUG_PORT           0xA
#define PCI_CAP_BASE_COMPACT_PCI_RES_CTL  0xB
#define PCI_CAP_BASE_PCI_HOTPLUG          0xC
#define PCI_CAP_BASE_PCI_AGP_8X           0xE
#define PCI_CAP_BASE_SECURE_DEVICE        0xF
#define PCI_CAP_BASE_PCIE                 0x10
#define PCI_CAP_BASE_MSIX                 0x11
#define PCI_CAP_BASE_SATA_DATA_IDX_CONFIG 0x12
#define PCI_CAP_BASE_AF                   0x13
#define PCI_CAP_BASE_ENHANCED_ALLOC       0x14
#define PCI_CAP_BASE_FLAT_PORTAL_BRIDGE   0x15

typedef struct pci_device pci_device;

typedef struct lst pci_device_lst;

struct pci_device {
    pci_device *prev;
    pci_device *next;
    u8 bus_nbr;
    u8 device_nbr;
    u8 function_nbr;
    pci_common_header header;
    pci_cap_lst capabilities;

    uintptr_t phys_addr;
    void *mapped_addr;
};

extern pci_device_lst pci_devices;
extern char **pci_class_name[];

void init_pci();
bool pci_is_multifunction(pci_device *);
void *pci_map_device(pci_device *);
void pci_unmap_device(pci_device *);

pci_cap *pci_find_cap(pci_device *, u16 cap_id, u16 cap_type);
#define PCI_CAP_WANTED_BASE 0x1
#define PCI_CAP_WANTED_EXT  0x2
void pci_scan_cap(pci_device *, u8 wanted);

#endif  // PCIE_H
