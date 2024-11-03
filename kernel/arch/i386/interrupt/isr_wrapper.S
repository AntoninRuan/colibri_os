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
    VECTOR_NO_ERR 0             // Divide by zero error
    VECTOR_NO_ERR 1             // Debug
    VECTOR_NO_ERR 2             // Non maskable interrupt
    VECTOR_NO_ERR 3             // Breakpoint
    VECTOR_NO_ERR 4             // Overflow
    VECTOR_NO_ERR 5             // Bound range exceeded
    VECTOR_NO_ERR 6             // Invalid Opcode
    VECTOR_NO_ERR 7             // Device not available
    VECTOR_ERR    8             // Double fault (err always 0)
    VECTOR_NO_ERR 9             // Unused (was x86 segment overrun)
    VECTOR_ERR    10            // Invalid TSS
    VECTOR_ERR    11            // Segment not present
    VECTOR_ERR    12            // Stack segment fault
    VECTOR_ERR    13            // General protection
    VECTOR_ERR    14            // Page fault
    VECTOR_NO_ERR 15            // Currently unused
    VECTOR_NO_ERR 16            // x87 FPU error
    VECTOR_ERR    17            // Alignement check
    VECTOR_NO_ERR 18            // Machine check
    VECTOR_NO_ERR 19            // SIMD (SSE/AVX) error
    // Currently unused
    VECTOR_NO_ERR 20
    VECTOR_NO_ERR 21
    VECTOR_NO_ERR 22
    VECTOR_NO_ERR 23
    VECTOR_NO_ERR 24
    VECTOR_NO_ERR 25
    VECTOR_NO_ERR 26
    VECTOR_NO_ERR 27
    VECTOR_NO_ERR 28
    VECTOR_NO_ERR 29
    VECTOR_NO_ERR 30
    VECTOR_NO_ERR 31
    // End of reserved vectors
