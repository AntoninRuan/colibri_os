.section.init.global _init.type _init @function _init : push % ebp movl % esp,
    %
        ebp
            // gcc will put the content of crtbegin.o's _init here

            .section.fini.global _fini.type _fini @function _fini : push %
        ebp movl % esp,
    % ebp
        // gcc wil put the content of crtbegin.o's _fini here
