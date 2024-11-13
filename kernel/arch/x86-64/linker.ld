ENTRY(_start)

SECTIONS
{
	. = 2M;

    _kernel_virtual_offset = 0xFFFFFFFF80000000;
    /* _kernel_virtual_addr = 0; */

    .pmode.text : {
        *(.multiboot.text)
        *(.text32)
        *(.data32)
    }

    . += _kernel_virtual_offset;
    _kernel_virtual_start = .;
	.text ALIGN(4K) : AT(ADDR(.text) - _kernel_virtual_offset)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - _kernel_virtual_offset)
	{
		*(.rodata)
        *(.rodata.*)
	}

	/* Read-write data (initialized) */
	.data ALIGN(4K) : AT(ADDR(.data) - _kernel_virtual_offset)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) : AT(ADDR(.bss) - _kernel_virtual_offset)
	{
		*(COMMON)
		*(.bss)
	}
    _kernel_virtual_end = .;

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}
