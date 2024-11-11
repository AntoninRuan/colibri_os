    .section .init
    .global _init
    .type _init @function
_init:
    pushq %rbp
    movq %rsp, %rbp
    // gcc will put the content of crtbegin.o's _init here

    .section .fini
    .global _fini
    .type _fini @function
_fini:
    pushq %rbp
    movq %rsp, %rbp
    // gcc wil put the content of crtbegin.o's _fini here
