#define __is_asm
#include <kernel/multiboot2.h>

    // Declare multiboot header
    .section .multiboot
    .align 8
multiboot_header:
    .long MULTIBOOT_MAGIC       // Magic
    .long MULTIBOOT_ARCH_I386   // Arch: i386
    .long multiboot_header_end - multiboot_header // Header length
    .long -(MULTIBOOT_MAGIC + MULTIBOOT_ARCH_I386 + (multiboot_header_end - multiboot_header)) // Checksum
// framebuffer_tag:
//     .long MULTIBOOT_HEADER_TAG_FRAMEBUFFER
//     .short 0
//     .long framebuffer_tag_end - framebuffer_tag
//     .long 1024                  // Width
//     .long 768                   // Height
//     .long 32                    // bits per pixel
// framebuffer_tag_end:
    .long MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:

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

    pushl $0
    popf

    pushl %ebx
    pushl %eax

    // Call global constructors
    // https://wiki.osdev.org/Calling_Global_Constructors
    call _init

    // Transfer control to kernel.c
    call main

    // Hang if kernel_main returns. (Should not happen)
    cli
1:  hlt
    jmp 1b

    .size _start, . - _start
