#include <elf.h>
#include <stddef.h>
#include <sys/cdefs.h>

char *get_elf_string(Elf64_Ehdr *header, u64 off) {
    if (header->e_shstrndx == SHN_UNDEF) return NULL;

    Elf64_Shdr *string_table = get_section(header, header->e_shstrndx);
    if (off > string_table->size) return NULL;

    return (char *)((void *)header + string_table->sh_offset + off);
}

Elf64_Shdr *get_section(Elf64_Ehdr *header, u64 idx) {
    u64 section_count = header->e_shnum;

    if (idx > section_count) return NULL;

    Elf64_Shdr *shdr_table = (Elf64_Shdr *)((void *)header + header->e_shoff);
    return &shdr_table[idx];
}

Elf64_Phdr *get_segment(Elf64_Ehdr *header, u64 idx) {
    u64 seg_count = header->e_phnum;

    if (idx > seg_count) return NULL;

    Elf64_Phdr *phdr_table = (Elf64_Phdr *)((void *)header + header->e_phoff);
    return &phdr_table[idx];
}

unsigned long elf64_has(const unsigned char *name) {
    unsigned long h = 0, g;

    while (*name) {
        h = (h << 4) + *name++;
        if ((g = h & 0xf0000000)) h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}
