    // Declare constant for multiboot header
    .set ALIGN,    1<<0               // align module on page
    .set MEMINFO,  1<<1             // provide memory map
    .set FLAGS,    ALIGN | MEMINFO
    .set MAGIC,    0x1BADB002
    .set CHECKSUM, -(MAGIC + FLAGS)

    // Declare multiboot header
    .section .multiboot
    .align 4
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

    // Reserve stack space for initial thread
    .section .bss
    .align 16
stack_bottom:
    .skip 16384
stack_top:

    // Kernel entry point
    .section .text
    .global _start
    .type _start @function
_start:
    movl $stack_top, %esp

    // Call global constructors
    // https://wiki.osdev.org/Calling_Global_Constructors
    call _init

    // Transfer control to kernel.c
    call main

    // Hang if kernel_main returns. (Should not happen)
    call main
    cli
1:  hlt
    jmp 1b

    .size _start, . - _start
