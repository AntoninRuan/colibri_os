#include <kernel/acpi.h>
#include <kernel/log.h>
#include <kernel/memory/memory_layout.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct rsdt *rsdt = 0;
struct xsdt *xsdt = 0;

bool validate_sdt(struct acpi_sdt_header *header) {
    u32 sum = 0;
    u8 *byte = (u8 *)header;
    for (u32 i = 0; i < header->length; i++) {
        sum += byte[i];
    }

    return !(sum & 0xFF);
}

// rsdt is a physical address, using the direct mapping to store it
// The address it contains are still physical one
// They need to be converted when they are read
int load_rsdp(struct rsdp *rsdp) {
    u32 sum = 0;
    u8 *byte = (u8 *)rsdp;
    for (size_t i = 0; i < sizeof(struct rsdp); i++) {
        sum += byte[i];
    }

    if (sum & 0xFF) return 1;

    rsdt = (struct rsdt *)((u64)rsdp->rsdt_addr + PHYSICAL_OFFSET);

    logf(INFO, "Loading rsdt at address 0x%X", rsdp->rsdt_addr);

    return 0;
}

// Same remark as for rsdt
int load_xsdp(struct xsdp *xsdp) {
    if (load_rsdp((struct rsdp *)xsdp)) return 1;

    if (xsdp->revision == 0) return 0;  // xsdp is actually a rsdp

    u32 sum = 0;
    u8 *byte = (u8 *)xsdp;
    for (size_t i = 0; sizeof(struct xsdp); i++) sum += byte[i];

    if (sum & 0xFF) return 1;

    xsdt = (struct xsdt *)((u64)xsdp->xsdt_address + PHYSICAL_OFFSET);

    logf(INFO, "Loading xsdt at address 0x%X", xsdp->xsdt_address);

    return 0;
}

// Address read in a sdt are physical address
// Returning the point in the direct mapping of phyiscal memory
struct acpi_sdt_header *find_table_in_rsdt(char *signature) {
    if (rsdt == NULL) return NULL;

    u32 entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    for (u32 i = 0; i < entries; i++) {
        uintptr_t header_addr =
            (uintptr_t)rsdt->sdt_addresses[i] + PHYSICAL_OFFSET;
        struct acpi_sdt_header *header = (struct acpi_sdt_header *)header_addr;
        if (is_acpi_table(header, signature)) return header;
    }

    return NULL;
}

struct acpi_sdt_header *find_table_in_xsdt(char *signature) {
    if (xsdt == NULL) return NULL;

    u32 entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;
    for (u32 i = 0; i < entries; i++) {
        uintptr_t header_addr =
            (uintptr_t)xsdt->sdt_addresses[i] + PHYSICAL_OFFSET;
        struct acpi_sdt_header *header = (struct acpi_sdt_header *)header_addr;
        if (is_acpi_table(header, signature)) return header;
    }

    return NULL;
}

struct acpi_sdt_header *find_table(char *signature) {
    if (xsdt != NULL) {
        return find_table_in_xsdt(signature);
    }

    return find_table_in_rsdt(signature);
}
