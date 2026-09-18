#include <kernel/driver/ahci.h>
#include <kernel/driver/pci.h>
#include <kernel/driver/sata.h>
#include <kernel/log.h>
#include <kernel/memory/heap.h>
#include <kernel/memory/physical_allocator.h>
#include <kernel/memory/vm.h>
#include <kernel/memory/vmm.h>
#include <sys/cdefs.h>

void do_bios_handoff(ahci_device *dev) {
    // TODO
}

void stop_port(hba_port *port) {
    port->cmd &= ~HBA_PXCMD_ST;   // Disable CMD list processing
    port->cmd &= ~HBA_PXCMD_FRE;  // Disable FIS receive

    while (1) {
        if (port->cmd & HBA_PXCMD_FR) continue;
        if (port->cmd & HBA_PXCMD_CR) continue;
        break;
    }
}

void start_port(hba_port *port) {
    // Wait for CMD_CR to be cleared
    while (port->cmd & HBA_PXCMD_CR);

    port->cmd |= HBA_PXCMD_FRE;
    port->cmd |= HBA_PXCMD_ST;
}

int remap_port(ahci_device *dev, hba_port *port, u16 port_no) {
    stop_port(port);

    // Get physical memory for port cmd headers and fis
    u64 phys = (u64)kalloc();
    if (!phys)
        // allocation failed, stop
        return 1;

    void *addr = map_mmio(NULL, phys, 0x400, true, false);
    if (!addr) {
        kfree((void *)phys);

        return 1;
    }

    port->clb = (u32)phys;
    port->clbu = 0;
    (dev->port_mem)[port_no].cmd_headers = (hba_command_header *)addr;

    phys += 0x400;
    addr += 0x400;

    port->fb = (u32)phys;
    port->fbu = 0;
    (dev->port_mem)[port_no].fis = (hba_port_fis *)addr;

    start_port(port);

    return 0;
}

void clean_ports_until(ahci_device *dev, u16 last) {
    // TODO Implement
}

void remap_ports(ahci_device *dev) {
    u8 port_count = (dev->hba->cap & HBA_CAP_NP) + 1;
    for (u16 port = 0; port < port_count; port++) {
        // Test if port is implemented
        if (!(dev->hba->pi & (1 << port))) continue;

        if (remap_port(dev, &dev->hba->ports[port], port)) {
            // Allocation failed, clean previously allocated port
            clean_ports_until(dev, port);
            return;
        }
    }
}

int find_cmdslot(hba_port *port) {
    u32 slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++) {
        if ((slots & 1) == 0) return i;
        slots >>= 1;
    }
    return -1;
}

void read(ahci_device *dev, u32 port_no, u64 start, u32 count) {
    hba_port *port = &dev->hba->ports[port_no];
    port->is = 0;
    int slot = find_cmdslot(port);
    if (slot == -1) return;

    hba_command_header header = dev->port_mem[port_no].cmd_headers[slot];
    header.cfl = sizeof(FIS_REG_H2D) / sizeof(u32);
    header.write = 0;
    // We read at most 4K per prdt, each sector is 512 bytes
    // 8 sectors per prdt
    header.prdtl = 1;

    u64 buf_p = (u64)kalloc();
    if (!buf_p) return;
    void *buf = map_mmio(NULL, buf_p, 4096, true, true);

    u64 phys = (u64)kalloc();
    if (!phys) {
        kfree((void *)buf_p);
        return;
    }
    header.ctba = (u32)(phys & 0xFFFFFFFF);
    header.ctbau = (u32)(phys >> 32);

    hba_command_table *cmds =
        (hba_command_table *)map_mmio(NULL, phys, 4096, true, false);

    if (!cmds) {
        kfree((void *)buf_p);
        kfree((void *)phys);
        return;
    }

    cmds->prdts[0].dba = (u32)(buf_p & 0xFFFFFFFF);
    cmds->prdts[0].dbau = (u32)(buf_p >> 32);
    cmds->prdts[0].dbc = 4096 - 1;

    FIS_REG_H2D *fis = (FIS_REG_H2D *)(&cmds->fis_cmd);
    fis->fis_type = REG_H2D;
    fis->c = 1;
    fis->command = 0x25;  // READ_DMA_EXT

    fis->lba = (u32)(start & 0xFFFFFF);
    fis->lba2 = (u32)(start >> 24);

    fis->count = 1;

    u64 spin = 0;
    while ((port->tfd & 0x88) != 0 && spin < 100000000) {
        spin++;
    }
    if (spin == 100000000) return;

    port->ci = 1 << slot;

    spin = 0;
    while (1) {
        if ((port->ci & (1 << slot)) == 0) break;
        spin++;
        if (spin == 100000000) {
            log(DEBUG, "stuck while reading");
            return;
        }
    }

    if (port->is & HBA_PXIS_TFES) {
        // Read error;
        log(DEBUG, "error while reading");
    }
}

void enable_ahci() {
    // Search for AHCI in pci devices
    pci_device *cur;
    cur = (pci_device *)pci_devices.next;
    for (; cur != (pci_device *)&pci_devices; cur = cur->next) {
        if (cur->header.class == PCI_CLASS_MASS_STORAGE_CONTROLLER
            && cur->header.subclass == 0x6) {
            // Found a SATA Controller
            if (cur->header.prog_if != 0x1)
                // Device is not AHCI, skip
                continue;

            if (!pci_map_device(cur)) continue;

            pci_scan_cap(cur, PCI_CAP_WANTED_BASE);
            pci_cap *msi =
                pci_find_cap(cur, PCI_CAP_BASE_MSI, PCI_CAP_TYPE_BASE);

            // No MSI capabilities do not handle
            if (!msi) continue;

            pci_header_0 *header = (pci_header_0 *)cur->mapped_addr;

            hba_mem *hba = map_mmio(NULL, header->bar5, 0x1100, true, false);
            if (!hba)
                // Error occured while mapping
                continue;

            ahci_device *ahci = alloc(sizeof(ahci_device));
            if (!ahci) {
                // TODO cleanup
                continue;
            }

            ahci->hba = hba;
            if (hba->cap2 & HBA_CAP2_BOH) {
                do_bios_handoff(ahci);
            }

            remap_ports(ahci);

            read(ahci, 0, 0, 1);

            log(DEBUG, "debug");
        }
    }
}
