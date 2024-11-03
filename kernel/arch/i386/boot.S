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
framebuffer_tag:
    .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
    .short MULTIBOOT_HEADER_FLAG_OPTIONAL
    .long framebuffer_tag_end - framebuffer_tag
    .long 0                  // Width
    .long 0                   // Height
    .long 0                    // bits per pixel
framebuffer_tag_end:
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

    // Reset EFLAGS
    pushl $0
    popf

    // Save multiboot magic value and address of multiboot2 info struct
    pushl %ebx
    pushl %eax

    cli
    call pre_main

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
