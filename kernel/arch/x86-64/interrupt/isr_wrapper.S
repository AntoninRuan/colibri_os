.macro push_xmm xmm
    sub $16, %rsp
    movdqu %xmm\xmm, (%rsp)
.endm

.macro pop_xmm xmm
    movdqu (%rsp), %xmm\xmm
    add $16, %rsp
.endm

.macro push_reg
    push_xmm 15
    push_xmm 14
    push_xmm 13
    push_xmm 12
    push_xmm 11
    push_xmm 10
    push_xmm 9
    push_xmm 8
    push_xmm 7
    push_xmm 6
    push_xmm 5
    push_xmm 4
    push_xmm 3
    push_xmm 2
    push_xmm 1
    push_xmm 0
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rsp
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %cr2, %rax
    pushq %rax
.endm

.macro pop_reg
    popq %rax
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rsp
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    pop_xmm 0
    pop_xmm 1
    pop_xmm 2
    pop_xmm 3
    pop_xmm 4
    pop_xmm 5
    pop_xmm 6
    pop_xmm 7
    pop_xmm 8
    pop_xmm 9
    pop_xmm 10
    pop_xmm 11
    pop_xmm 12
    pop_xmm 13
    pop_xmm 14
    pop_xmm 15
.endm

    .text
isr_wrapper:
    push_reg

    movq %rsp, %rdi             // Pass stack addr as an args for function call
    call interrupt_dispatch
    mov %rax, %rsp              // Restore %rsp to what it was

    pop_reg

    add $16, %rsp               // remove vector number + error code

    iretq

.macro vector_no_err vec_num
    .align 16
    .global vector_handler_\vec_num
vector_handler_\vec_num:
    // Add a dummy error to align the case with error code
    pushq $0
    pushq $\vec_num

    jmp isr_wrapper
.endm

.macro vector_err vec_num
    .align 16
    .global vector_handler_\vec_num
vector_handler_\vec_num:
    // Already have an error code no need to add a dummy
    pushq $\vec_num

    jmp isr_wrapper
.endm

    // Reserved vectors
    vector_no_err 0             // Divide by zero error
    vector_no_err 1             // Debug
    vector_no_err 2             // Non maskable interrupt
    vector_no_err 3             // Breakpoint
    vector_no_err 4             // Overflow
    vector_no_err 5             // Bound range exceeded
    vector_no_err 6             // Invalid Opcode
    vector_no_err 7             // Device not available
    vector_err    8             // Double fault (err always 0)
    vector_no_err 9             // Unused (was x86 segment overrun)
    vector_err    10            // Invalid TSS
    vector_err    11            // Segment not present
    vector_err    12            // Stack segment fault
    vector_err    13            // General protection
    vector_err    14            // Page fault
    vector_no_err 15            // Currently unused
    vector_no_err 16            // x87 FPU error
    vector_err    17            // Alignement check
    vector_no_err 18            // Machine check
    vector_no_err 19            // SIMD (SSE/AVX) error
    // Currently unused
    vector_no_err 20
    vector_no_err 21
    vector_no_err 22
    vector_no_err 23
    vector_no_err 24
    vector_no_err 25
    vector_no_err 26
    vector_no_err 27
    vector_no_err 28
    vector_no_err 29
    vector_no_err 30
    vector_no_err 31
    // End of reserved vectors

    // IRQs
    vector_no_err 0x21          // Keyboard interrupt

    // HPET timers
    vector_no_err 0x30
    vector_no_err 0x31
    vector_no_err 0x32
    vector_no_err 0x33
    vector_no_err 0x34
    vector_no_err 0x35
    vector_no_err 0x36
    vector_no_err 0x37
    vector_no_err 0x38
    vector_no_err 0x39
    vector_no_err 0x3A
    vector_no_err 0x3B
    vector_no_err 0x3C
    vector_no_err 0x3D
    vector_no_err 0x3E
    vector_no_err 0x3F
    // APIC timer
    vector_no_err 0x40

    vector_no_err 0xFF          // APIC Spurious vector
