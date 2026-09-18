#ifndef SATA_H
#define SATA_H

#include <sys/cdefs.h>

typedef enum {
    REG_H2D = 0x27,
    REG_D2H = 0x34,
    DMA_ACT = 0x39,
    DMA_SETUP = 0x41,
    DATA = 0x46,
    BIST = 0x58,
    PIO_SETUP = 0x5F,
    DEV_BITS = 0xA1,
} FIS_TYPE;

typedef struct {
    // DW 0
    u8 fis_type;
    u8 pmport : 4;
    u8        : 3;
    u8 c      : 1;  // 1 Command, 0: Control

    u8 command;
    u8 featurel;

    // DW 1
    u32 lba    : 24;
    u32 device : 8;

    // DW 2
    u32 lba2     : 24;
    u32 featureu : 8;

    // DW 3
    u16 count;
    u8 icc;
    u8 control;

    // DW 4
    u32 rsv;
} FIS_REG_H2D;

typedef struct {
    // DW 0
    u8 fis_type;
    u8 pmport : 4;
    u8        : 2;
    u8 i      : 1;
    u8        : 1;
    u8 status;
    u8 error;

    // DW 1
    u32 lba1   : 24;
    u32 device : 8;

    // DW 2
    u32 lba2 : 24;
    u32      : 8;

    // DW 3 and 4
    u16 count;
    u8 rsv[6];
} FIS_REG_D2H;

typedef struct {
    // DW 0
    u8 fis_type;
    u8 pmport : 4;
    u8        : 4;

    u16 rsv;

    u32 *data;
} FIS_DATA;

typedef struct {
    // DW 0
    u8 fis_type;
    u8 pmport : 4;
    u8        : 1;
    u8 d      : 1;
    u8 i      : 1;
    u8        : 1;

    u8 status;
    u8 error;

    // DW 1
    u32 lba1   : 24;
    u32 device : 8;

    // DW 2
    u32 lba2 : 24;
    u32      : 8;

    // DW 3
    u16 coun;
    u8 rsv;
    u8 new_status;

    // DW 4
    u16 transfer_count;
    u16 rsv1;
} FIS_PIO_SETUP;

typedef struct {
    u8 fis_type;
    u8 pmport : 4;
    u8        : 1;
    u8 d      : 1;
    u8 i      : 1;
    u8 a      : 1;
    u16 rsv;

    u64 buffer_id;

    u32 rsvd;

    u32 buffer_offset;

    u32 transfer_count;

    u32 rsv1;
} FIS_DMA_SETUP;

#endif  // SATA_H
