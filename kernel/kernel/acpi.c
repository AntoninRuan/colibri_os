#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/acpi.h>

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

int load_xsdp(struct xsdp *xsdp) {
    if (load_rsdp((struct rsdp *) xsdp)) return 1;

    if (xsdp->revision == 0) return 0; // xsdp is actually a rsdp

    uint32_t sum = 0;
    uint8_t *byte = (uint8_t *) xsdp;
    for (size_t i = 0; sizeof(struct xsdp); i ++)
        sum += byte[i];

    if (sum & 0xFF) return 1;

    xsdt = (struct xsdt *) xsdp->xsdt_address;

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
