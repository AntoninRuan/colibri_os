    .section .init
    // gcc will put the content of crtend.o's _init here.
    popl %ebp
    ret

    .section .fini
    // gcc will put the content of crtend.os' _fini here.
    popl %ebp
    ret
