#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/acpi.h>
#include <kernel/arch/x86-64/vm.h>

struct rsdt *rsdt = 0;
struct xsdt *xsdt = 0;

bool validate_sdt(struct acpi_sdt_header *header) {
    uint32_t sum = 0;
    uint8_t *byte = (uint8_t *) header;
    for (uint32_t i = 0; i < header->length; i ++) {
        sum += byte[i];
    }

    return !(sum & 0xFF);
}

// rsdt is a physical address, everything informations
// needed must be read before the unmapping of the identiy mapping
// used during the long jump. rsdt could actually be mapped but
// the address it contains would still be physical one
// If it is needed to read more informations after boot, it will
// be needed to map it and every address it contains and translate
// as they are needed
int load_rsdp(struct rsdp *rsdp) {
    uint32_t sum = 0;
    uint8_t *byte = (uint8_t *) rsdp;
    for (size_t i = 0; i < sizeof(struct rsdp); i ++) {
        sum += byte[i];
    }

    if (sum & 0xFF) return 1;

    rsdt = (struct rsdt *) rsdp->rsdt_addr;

    return 0;
}

// Same remark as for rsdt
int load_xsdp(struct xsdp *xsdp) {
    if (load_rsdp((struct rsdp *) xsdp)) return 1;

    if (xsdp->revision == 0) return 0; // xsdp is actually a rsdp

    uint32_t sum = 0;
    uint8_t *byte = (uint8_t *) xsdp;
    for (size_t i = 0; sizeof(struct xsdp); i ++)
        sum += byte[i];

    if (sum & 0xFF) return 1;

    xsdt = (struct xsdt *) xsdp->xsdt_address;
    // xsdt = (struct xsdt *) map_mmio(xsdp->xsdt_addr, sizeof(struct xsdp), false);

    return 0;
}

struct acpi_sdt_header *find_table_in_rsdt(char *signature) {
    if (rsdt == NULL) return NULL;

    uint32_t entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;
    for (uint32_t i = 0; i < entries; i ++) {
        struct acpi_sdt_header *header = (struct acpi_sdt_header *) rsdt->sdt_addresses[i];
        if (is_acpi_table(header, signature)) return header;
    }

    return NULL;
}

struct acpi_sdt_header *find_table_in_xsdt(char *signature) {
    if (xsdt == NULL) return NULL;

    uint32_t entries = (xsdt->header.length - sizeof(xsdt->header)) / 8;
    for (uint32_t i = 0; i < entries; i ++) {
        struct acpi_sdt_header *header = (struct acpi_sdt_header *) xsdt->sdt_addresses[i];
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
