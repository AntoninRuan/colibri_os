    .text
    .global flush_gdt
flush_gdt:
    jmp $0x08, $reloadCS

reloadCS:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret
