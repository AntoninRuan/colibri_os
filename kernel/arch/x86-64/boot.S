#include <kernel/multiboot2.h>
#include <kernel/kernel.h>

    // Declare multiboot header
    .section .multiboot.text, "a"
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

    .section .bss, "aw", @nobits
    .align 16
stack_bottom:
    .skip 16384 * MAX_CORES
stack_top:

    // Kernel entry point
    .section .text32, "ax"
    .global _start
    .type _start @function
    .code32
_start:
    // Still in Protected Mode (32 bit)

    // Multiboot2 loader must have already disabled paging and interrupts

    // Save multiboot magic value and address of multiboot2 info struct
    movl %ebx, %esi
    movl %eax, %edi

#define KERNEL_VMA 0xFFFFFFFF80000000
#define PRESENT 1
#define WRITABLE 1 << 1
#define PS_BIT 1 << 7

    // Map pml4 onto itself
    movl $(pml4 - KERNEL_VMA), %eax
    orl $(PRESENT | WRITABLE), %eax
    movl %eax, (pml4 - KERNEL_VMA + 510 * 8)

    // Prepare higher half kernel mapping
    movl $(kernel_p3_hh - KERNEL_VMA), %eax
    orl $(PRESENT | WRITABLE), %eax
    movl %eax, pml4 - KERNEL_VMA + 511 * 8

    // Prepare identity mapping for jump in long mode
    movl $(kernel_p3 - KERNEL_VMA), %eax
    orl $(PRESENT | WRITABLE), %eax
    movl %eax, pml4 - KERNEL_VMA + 0 * 8

    // Actual mapping of the kernel
    movl $(kernel_p2 - KERNEL_VMA), %eax
    orl $(PRESENT | WRITABLE), %eax

    movl %eax, kernel_p3 - KERNEL_VMA + 0 * 8
    movl %eax, kernel_p3_hh - KERNEL_VMA + 510 * 8

    mov $0, %ecx
fill_p2:
    movl $0x200000, %eax
    mull %ecx
    orl $(PRESENT | WRITABLE | PS_BIT), %eax

    movl %eax, kernel_p2 - KERNEL_VMA(,%ecx, 8)

    incl %ecx
    cmpl $512, %ecx

    jne fill_p2

    // Default paging is configured

    // Get local APIC ID (CPUID.1:EBX[24..32])
    mov $1, %eax
    cpuid
    shrl $24, %ebx
    movl %ebx, %ebx

    .global ap_boot
ap_boot:
    // Control register can only be written from gp reg
    // Enabling Physical Address Extension
    movl %cr4, %eax
    orl $(1<<5), %eax
    movl %eax, %cr4

    // Load cr3 with physical address of pml4
    movl $(pml4 - KERNEL_VMA), %eax
    movl %eax, %cr3

    // Enable long mode
    // Setting bit 8 in EFER MSR (0xC0000080)
    movl $0xC0000080, %ecx
    rdmsr                       // Reading in edx:eax
    orl $(1 << 8), %eax
    wrmsr                       // Writing from edx:eax

    lgdt (gdtr_low - KERNEL_VMA)

    movl %cr0, %eax
    orl $(1<<31), %eax          // Enable paging
    orl $(1<<16), %eax          // Enable Write protect
    movl %eax, %cr0

    jmp $0x8, $(long_mode_jump - KERNEL_VMA)

    .code64
    .section .text
long_mode_jump:
    cli
    // Update segment selector
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    lea kernel64, %rax
    jmpq *%rax

kernel64:
    // Setup stack 16k for each core
    movl %ebx, %eax
    shlq $14, %rax
    movq $stack_top, %rsp
    subq %rax, %rsp


    // Update gdt to read in higher half mapped memory
    lgdt gdtr_high

    // Enable SSE extension
    movq %cr4, %rax
    orq $1 << 9, %rax           // set OSFXSR Flag
    // TODO add #XM handler
    // orq $1 << 10, %rax          // set OSXMMEXCPT Flag
    movq %rax, %cr4

    movq %cr0, %rax
    andq $~(1<<2), %rax         // clear EM flag
    orq $1<<1, %rax             // set MP flag
    movq %rax, %cr0

bsp_detection:
    // We want to check if we are booting the first cpu (the BSP
    // Or another one (an AP)
    xorq %rax, %rax
    movl kernel_status, %eax
    cmp $0, %eax
    je bsp_init

ap_init:
    movq %rbx, %rdi
    call ap_startup

bsp_init:
    movq %rbx, %rdx
    call bsp_startup

noSSE:
    // Hang if kernel_main returns. (Should not happen)
    cli
1:  hlt
    jmp 1b

    .section .bss, "aw", @nobits
    .global pml4
    .global kernel_p3_hh
    .align 4096
pml4:
    .skip 4096

kernel_p3:
    .skip 4096

kernel_p3_hh:
    .skip 4096

kernel_p2:
    .skip 4096

#define FLAG_G          (1 << 55)
#define FLAG_DB         (1 << 54)
#define FLAG_L          (1 << 53)

#define ACCESS_PRESENT  (1 << 47)
#define ACCESS_DPL(x)   (x << 45)
#define ACCESS_S        (1 << 44)
#define ACCESS_CODE     (1 << 43)
#define ACCESS_DC       (1 << 42)
#define ACCESS_RW       (1 << 41)
#define ACCESS_A        (1 << 40)

    .section .data
    // GDT is using a flat model with 5 entry
    // the first one being the mandatory null one
    // 2 for kernel code and data
    // 2 for user code and data
    .global gdtr64
    .global gdtr32
gdt64:
    .quad 0

#define FLAGS (FLAG_G | FLAG_L)
#define ACCESS_KERNEL (ACCESS_PRESENT | ACCESS_DPL(0) | ACCESS_S | ACCESS_RW | ACCESS_A)
#define ACCESS_USER (ACCESS_KERNEL | ACCESS_DPL(3))
    // Limit will always be 0xFFFFF
    // So bits 0..15 must be set
    // And also bits 48..51
#define LIMIT (0xFFFF | (0xF << 48))

    .quad (FLAGS | ACCESS_KERNEL | ACCESS_CODE | LIMIT)
    .quad (FLAGS | ACCESS_KERNEL | LIMIT)
    .quad (FLAGS | ACCESS_USER | ACCESS_CODE | LIMIT)
    .quad (FLAGS | ACCESS_USER | LIMIT)

    // Add placeholder for task state segment
    .quad 0
    .quad 0

gdtr_low:
    .short gdtr_low - gdt64 - 1
    .long (gdt64 - KERNEL_VMA)

gdtr_high:
    .short gdtr_low - gdt64 - 1
    .quad gdt64
