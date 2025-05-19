#ifndef ACPI_H
#define ACPI_H

#include <stdbool.h>
#include <sys/cdefs.h>

// https://uefi.org/sites/default/files/resources/ACPI_6_3_final_Jan30.pdf#G10.1368542
#define ACPI_TABLE_APIC "APIC"
#define ACPI_TABLE_BERT "BERT"
#define ACPI_TABLE_BGRT "BGRT"
#define ACPI_TABLE_CPEP "CPEP"
#define ACPI_TABLE_DSDT "DSDT"
#define ACPI_TABLE_ECDT "ECDT"
#define ACPI_TABLE_EINJ "EINJ"
#define ACPI_TABLE_ERST "ERST"
#define ACPI_TABLE_FACP "FACP"
#define ACPI_TABLE_FACS "FACS"
#define ACPI_TABLE_FPDT "FPDT"
#define ACPI_TABLE_GTDT "GTDT"
#define ACPI_TABLE_HEST "HEST"
#define ACPI_TABLE_MSCT "MSCT"
#define ACPI_TABLE_MPST "MPST"
#define ACPI_TABLE_NFIT "NFIT"
#define ACPI_TABLE_PCCT "PCCT"
#define ACPI_TABLE_PMTT "PMTT"
#define ACPI_TABLE_PSDT "PSDT"
#define ACPI_TABLE_RAFS "RAFS"
#define ACPI_TABLE_RSDT "RSDT"
#define ACPI_TABLE_SBST "SBST"
#define ACPI_TABLE_SDEV "SDEV"
#define ACPI_TABLE_SLIT "SLIT"
#define ACPI_TABLE_SRAT "SRAT"
#define ACPI_TABLE_SSDT "SSDT"
#define ACPI_TABLE_XSDT "XSDT"

#define is_acpi_table(header, type) !(memcmp(header->signature, type, 4))

struct rsdp {
    char signature[8];  // Not null terminated
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
} __attribute__((packed));

struct xsdp {
    char signature[8];  // Not null terminated
    u8 checksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;  // Deprecated in xsdp

    u32 length;
    u64 xsdt_address;
    u8 extented_checksum;
    u8 reserved[3];
} __attribute__((packed));

struct acpi_sdt_header {
    char signature[4];  // Not null terminated
    u32 length;
    u8 revision;
    u8 checksum;
    char oem_id[6];
    char oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
};

typedef struct acpi_sdt_header acpi_sdt_header_t;

struct rsdt {
    acpi_sdt_header_t header;
    u32 sdt_addresses[];
};

struct xsdt {
    acpi_sdt_header_t header;
    u64 sdt_addresses[];
};

int load_rsdp(struct rsdp *);
int load_xsdp(struct xsdp *);

bool validate_sdt(acpi_sdt_header_t *);
acpi_sdt_header_t *find_table(char *signature);

#endif  // ACPI_H
