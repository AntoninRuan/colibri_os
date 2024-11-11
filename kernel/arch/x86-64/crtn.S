    .section .init
    // gcc will put the content of crtend.o's _init here.
    popq %rbp
    ret

    .section .fini
    // gcc will put the content of crtend.os' _fini here.
    popq %rbp
    ret
