    .text
isr_wrapper:
    pushal

    pushl %esp                  // Pass stack addr as an args for function call
    call interrupt_dispatch
    mov %eax, %esp              // Restore %esp to what it was

    popal

    add $8, %esp               // remove vector number + error code

    iret

.macro vector_no_err vec_num
    .align 16
    .global vector_handler_\vec_num
vector_handler_\vec_num:
    // Add a dummy error to align the case with error code
    pushl $0
    pushl $\vec_num

    jmp isr_wrapper
.endm

.macro vector_err vec_num
    .align 16
    .global vector_handler_\vec_num
vector_handler_\vec_num:
    // Already have an error code no need to add a dummy
    pushl $\vec_num

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

    // APIC Spurious vector
    vector_no_err 0xFF
