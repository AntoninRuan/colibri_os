#ifndef ACHI_H
#define ACHI_H

#include <kernel/driver/sata.h>
#include <sys/cdefs.h>

#define HBA_PXIS_DHRS (0x00000001)
#define HBA_PXIS_PSS  (0x00000002)
#define HBA_PXIS_DSS  (0x00000004)
#define HBA_PXIS_SDBS (0x00000008)
#define HBA_PXIS_UFS  (0x00000010)
#define HBA_PXIS_DPS  (0x00000020)
#define HBA_PXIS_PCS  (0x00000040)
#define HBA_PXIS_DMPS (0x00000080)
#define HBA_PXIS_PRCS (0x00400000)
#define HBA_PXIS_IMPS (0x00800000)
#define HBA_PXIS_OFS  (0x01000000)
#define HBA_PXIS_INFS (0x04000000)
#define HBA_PXIS_IFS  (0x08000000)
#define HBA_PXIS_HBDS (0x10000000)
#define HBA_PXIS_HBFS (0x20000000)
#define HBA_PXIS_TFES (0x40000000)
#define HBA_PXIS_CPDS (0x80000000)

#define HBA_PXIE_DHRE (0x00000001)
#define HBA_PXIE_PSE  (0x00000002)
#define HBA_PXIE_DSE  (0x00000004)
#define HBA_PXIE_SDBE (0x00000008)
#define HBA_PXIE_UFE  (0x00000010)
#define HBA_PXIE_DPE  (0x00000020)
#define HBA_PXIE_PCE  (0x00000040)
#define HBA_PXIE_DMPE (0x00000080)
#define HBA_PXIE_PRCE (0x00400000)
#define HBA_PXIE_IMPE (0x00800000)
#define HBA_PXIE_OFE  (0x01000000)
#define HBA_PXIE_INFE (0x04000000)
#define HBA_PXIE_IFE  (0x08000000)
#define HBA_PXIE_HBDE (0x10000000)
#define HBA_PXIE_HBFE (0x20000000)
#define HBA_PXIE_TFEE (0x40000000)
#define HBA_PXIE_CPDE (0x80000000)

#define HBA_PXCMD_ST    (0x00000001)
#define HBA_PXCMD_SUD   (0x00000002)
#define HBA_PXCMD_POD   (0x00000004)
#define HBA_PXCMD_CLO   (0x00000008)
#define HBA_PXCMD_FRE   (0x00000010)
#define HBA_PXCMD_CCS   (0x00001F00)
#define HBA_PXCMD_MPSS  (0x00002000)
#define HBA_PXCMD_FR    (0x00004000)
#define HBA_PXCMD_CR    (0x00008000)
#define HBA_PXCMD_CPS   (0x00010000)
#define HBA_PXCMD_PMA   (0x00020000)
#define HBA_PXCMD_HPCP  (0x00040000)
#define HBA_PXCMD_MPSP  (0x00080000)
#define HBA_PXCMD_CPD   (0x00100000)
#define HBA_PXCMD_ESP   (0x00200000)
#define HBA_PXCMD_FBSCP (0x00400000)
#define HBA_PXCMD_APSTE (0x00800000)
#define HBA_PXCMD_ATAPI (0x01000000)
#define HBA_PXCMD_DLAE  (0x02000000)
#define HBA_PXCMD_ALPE  (0x04000000)
#define HBA_PXCMD_ASP   (0x08000000)
#define HBA_PXCMD_ICC   (0xF0000000)

typedef volatile struct {
    u32 clb;
    u32 clbu;
    u32 fb;
    u32 fbu;
    u32 is;
    u32 ie;
    u32 cmd;
    u32 reserved0;
    u32 tfd;
    u32 sig;
    u32 ssts;
    u32 sctl;
    u32 serr;
    u32 sact;
    u32 ci;
    u32 sntf;
    u32 fbs;
    u32 devslp;
    u32 reserved1[11];
    u32 vs[4];
} hba_port;

#define HBA_CAP_NP    (0x0000001F)
#define HBA_CAP_SXS   (0x00000020)
#define HBA_CAP_EMS   (0x00000040)
#define HBA_CAP_CCCS  (0x00000080)
#define HBA_CAP_NCS   (0x00001F00)
#define HBA_CAP_PSC   (0x00002000)
#define HBA_CAP_SSC   (0x00004000)
#define HBA_CAP_PMD   (0x00008000)
#define HBA_CAP_FBSS  (0x00010000)
#define HBA_CAP_SPM   (0x00020000)
#define HBA_CAP_SAM   (0x00040000)
#define HBA_CAP_ISS   (0x00F00000)
#define HBA_CAP_SCLO  (0x01000000)
#define HBA_CAP_SAL   (0x02000000)
#define HBA_CAP_SALP  (0x04000000)
#define HBA_CAP_SSS   (0x08000000)
#define HBA_CAP_SMPS  (0x10000000)
#define HBA_CAP_SSNTF (0x20000000)
#define HBA_CAP_SNCQ  (0x40000000)
#define HBA_CAP_S64A  (0x80000000)

#define HBA_GHC_HR   (0x00000001)
#define HBA_GHC_IE   (0x00000002)
#define HBA_GHC_MRSM (0x00000004)
#define HBA_GHC_AE   (0x80000000)

#define HBA_VS_MINOR (0x0000FFFF)
#define HBA_VS_MAJOR (0xFFFF0000)

#define HBA_CCC_CTL_EN  (0x00000001)
#define HBA_CCC_CTL_INT (0x000000F8)
#define HBA_CCC_CTL_CC  (0x0000FF00)
#define HBA_CCC_CTL_TV  (0xFFFF0000)

#define HBA_EM_LOC_SZ   (0x0000FFFF)
#define HBA_EM_LOC_OFST (0xFFFF0000)

#define HBA_EM_CTL_MR    (0x00000001)
#define HBA_EM_CTL_TM    (0x00000100)
#define HBA_EM_CTL_RST   (0x00000200)
#define HBA_EM_CTL_LED   (0x00010000)
#define HBA_EM_CTL_SAFTE (0x00020000)
#define HBA_EM_CTL_SES2  (0x00040000)
#define HBA_EM_CTL_SGPIO (0x00080000)
#define HBA_EM_CTL_SMB   (0x01000000)
#define HBA_EM_CTL_XMT   (0x02000000)
#define HBA_EM_CTL_ALHD  (0x04000000)
#define HBA_EM_CTL_PM    (0x08000000)

#define HBA_CAP2_BOH  (0x00000001)
#define HBA_CAP2_NVMP (0x00000002)
#define HBA_CAP2_APST (0x00000004)
#define HBA_CAP2_SDS  (0x00000008)
#define HBA_CAP2_SADM (0x00000010)
#define HBA_CAP2_DESO (0x00000020)

#define HBA_BOHC_BOS  (0x01)
#define HBA_BOHC_OOS  (0x02)
#define HBA_BOHC_SOOE (0x04)
#define HBA_BOHC_OOC  (0x08)
#define HBA_BOHC_BB   (0x10)

typedef volatile struct {
    // 0x00-0x2B Generic Host Control
    u32 cap;
    u32 ghc;
    u32 is;
    u32 pi;
    u32 vs;
    u32 ccc_ctl;
    u32 ccc_ports;
    u32 em_loc;
    u32 em_ctl;
    u32 cap2;
    u32 bohc;

    // 0x2C-0xA0 reserved
    u8 rsv[0xA0 - 0x2C];

    // 0xA0 - 0xFF Vendor specific regsiter
    u8 vendor[0x100 - 0xA0];

    hba_port ports[32];
} hba_mem;

typedef struct {
    // DW 0 Description information
    u16 cfl          : 5;
    u16 atapi        : 1;
    u16 write        : 1;
    u16 prefetchable : 1;
    u16 reset        : 1;
    u16 bist         : 1;
    u16 clear        : 1;
    u16              : 1;
    u16 pmp          : 4;

    volatile u16 prdtl;

    // DW 1
    u32 cmd_status;

    // DW 2
    u32 ctba;

    // DW 3
    u32 ctbau;

    u32 reserved[4];
} hba_command_header;

typedef struct {
    u32 dba;  // Must be word aligned
    u32 dbau;
    u32 rsv;

    u32 dbc             : 22;
    u32                 : 9;
    u32 int_on_complete : 1;
} hba_prdt_entry;

typedef struct {
    u8 fis_cmd[64];
    u8 atapi_cmd[16];
    u8 rsv[48];
    hba_prdt_entry prdts[];
} __attribute__((packed)) hba_command_table;

typedef struct {
    FIS_DMA_SETUP dma_setup;
    u8 rsv0[4];
    FIS_PIO_SETUP pio_setup;
    u8 rsv1[12];
    FIS_REG_D2H rfis;
    u8 rsv2[4];
    u8 sdbfis[2];
    u8 ufis[64];
    u8 rsv3[0x5F];
} __attribute__((packed)) hba_port_fis;

typedef struct {
    hba_port_fis *fis;
    hba_command_header *cmd_headers;
    hba_command_table *cmd_table[32];
} __attribute__((packed)) hba_port_mem;

typedef struct {
    hba_mem *hba;

    hba_port_mem port_mem[32];
} __attribute__((packed)) ahci_device;

void enable_ahci();

#endif  // ACHI_H
