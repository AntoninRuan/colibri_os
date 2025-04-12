# Virtual Memory Layout

| Start address             | End address               | Size   | Description            |
|---------------------------|---------------------------|--------|------------------------|
| ``0x0000_0000_0000_0000`` | ``0x0000_7FFF_FFFF_FFFF`` | 128 Tb | Free                   |
|---------------------------|---------------------------|--------|------------------------|
| ``0xFFFF_8000_0000_0000`` | ``0xFFFF_FE7F_FFFF_FFFF`` | 126 Tb | Free                   |
| ``0xFFFF_FE80_0000_0000`` | ``0xFFFF_FEFF_FFFF_FFFF`` | 512 Gb | Physical Mapping       |
| ``0xFFFF_FF00_0000_0000`` | ``0xFFFF_FF7F_FFFF_FFFF`` | 512 Gb | PML4 recursive mapping |
| ``0xFFFF_FF80_0000_0000`` | ``0xFFFF_FFFF_7FFF_FFFF`` | 510 Gb | Free                   |
| ``0xFFFF_FFFF_8000_0000`` | ``0xFFFF_FFFF_BFFF_FFFF`` | 1 Gb   | Kernel mapping         |
| ``0xFFFF_FFFF_C000_0000`` | ``0xFFFF_FFFF_FFFF_FFFF`` | 1 Gb   | Free                   |
