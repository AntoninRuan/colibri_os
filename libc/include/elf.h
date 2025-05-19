#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;
typedef int32_t Elf64_Sword;
typedef u64 Elf64_Xword;
typedef int64_t Elf64_Sxword;

// ELF64 Header
typedef struct {
    unsigned char e_ident[16];  // ELF Identification
    Elf64_Half e_type;          // Object file type
    Elf64_Half e_machine;       // Machine type
    Elf64_Word e_version;       // Object file version
    Elf64_Addr e_entry;         // Entry point address
    Elf64_Off e_phoff;          // Program header offset
    Elf64_Off e_shoff;          // Section header offset
    Elf64_Word e_flags;         // Processor-specific flags
    Elf64_Half e_ehsize;        // ELF header size
    Elf64_Half e_phentisze;     // Size of program header entry
    Elf64_Half e_phnum;         // Number of program header entries
    Elf64_Half e_shentsize;     // Size of section header entry
    Elf64_Half e_shnum;         // Number of section header entries
    Elf64_Half e_shstrndx;      // Secion name string table index
} Elf64_Ehdr;

// ELF Identification, e_ident indexes
#define EI_MAG0    0  // File identification
#define EI_MAG1    1
#define EI_MAG2    2
#define EI_MAG3    3
#define EI_CLASS   4   // File class
#define EI_DATA    5   // Data encoding
#define EI_VERSION 6   // File version
#define EI_OSABI   7   // OS/ABI identification
#define EI_PAD     9   // Start of padding bytes
#define EI_NIDENT  16  // Size of e_ident[]

// Object file classes e_ident[EI_CLASS]
#define ELFCLASS32 1
#define ELFCLASS64 1

// Data Encodings e_ident[EI_DATA]
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

// Operating System and ABI identifiers, e_ident[EI_OSABI]
#define ELFOSABI_SYSV       0
#define ELFOSABI_HPUX       1
#define ELFOSABI_STANDALONE 255

// Object File Types
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOOS   0xFE00
#define ET_HIOS   0xFEFF
#define ET_LOPROC 0xFF00
#define ET_HIPROC 0xFFFF

// Special Section Indices
#define SHN_UNDEF  0       // Mark an undefined of meaningless section
#define SHN_LOPROC 0xFF00  // Processor-specific use
#define SHN_HIPROC 0xFF1F
#define SHN_LOOS   0xFF20  // Env specific use
#define SHN_HIOS   0xFF3F
#define SHN_ABS    0xFFF1  // Correspoding ref is absolute value
#define SHN_COMMON 0xFFF2  // Symbol that has been declared as a common block

// Section Header
typedef struct {
    Elf64_Word sh_name;        // Section name
    Elf64_Word sh_type;        // Section type
    Elf64_Xword sh_flags;      // Section attributes
    Elf64_Addr sh_addr;        // Virtual address in memory
    Elf64_Off sh_offset;       // Offset in file
    Elf64_Xword size;          // Size of section
    Elf64_Word sh_link;        // Link to other section
    Elf64_Word sh_info;        // Miscellaneous information
    Elf64_Xword sh_addralign;  // Address alignement boundary
    Elf64_Xword sh_entsize;    // Size of entries, if section has table
} Elf64_Shdr;

// Section Types
#define SHT_NULL      0
#define SHT_PROGBITS  1
#define SHT_SYMTAB    2
#define SHT_STRTAB    3
#define SHT_RELA      4
#define SHT_HASH      5
#define SHT_DYNAMIC   6
#define SHT_NOTE      7
#define SHT_NOBITS    8
#define SHT_REL       9
#define SHT_SHLIB     10
#define SHT_DYNSYM    11
#define SHT_LOOS      0x60000000
#define SHT_HIOS      0x6FFFFFFF
#define SHT_LOPROC    0x70000000
#define SHT_HIPROC    0x7FFFFFFF
#define SHT_WRITE     0x1
#define SHT_ALLOC     0x2
#define SHT_EXECINSTR 0x4
#define SHT_MASKOS    0x0F000000
#define SHT_MASKPROC  0xF0000000

// Symbol Table
typedef struct {
    Elf64_Word st_name;      // Symbol name
    unsigned char st_info;   // Type and binding attributes
    unsigned char st_other;  // Reserved
    Elf64_Half st_shndx;     // Section table index
    Elf64_Addr st_value;     // Symbol value
    Elf64_Xword st_size;     // Size f object
} Elf64_Sym;

// Symbol Bindings
#define STB_LOCAL  0  // Not visible outside object file
#define STB_GLOBAL 1  // Visible to all object files
#define STB_WEAK   2  // Global scope, with lower precedence than global symbols
#define STB_LOOS   10  // Environment specific use
#define STB_HIOS   12
#define STB_LOPROC 13  // Processor specific use
#define STB_HIPROC 15

// Symbol Types
#define STT_NOTYPE  0   // Absolute symbol
#define STT_OBJECT  1   // Data object
#define STT_FUNC    2   // Function entry point
#define STT_SECTION 3   // Associated with a section
#define STT_FILE    4   // Source file associated with the object file
#define STT_LOOS    10  // Environment specific use
#define STT_HIOS    12
#define STT_LOPROC  13  // Processor specific use
#define STT_HIPROC  15

// Relocations entries
typedef struct {
    Elf64_Addr r_offset;  // Address of reference
    Elf64_Xword r_info;   // Symbol index and type of relocation
} Elf64_Rel;

typedef struct {
    Elf64_Addr r_offset;    // Address of reference
    Elf64_Xword r_info;     // Symbol index and type of relocation
    Elf64_Sxword r_addend;  // Constant part of expression
} Elf64_Rela;

#define ELF64_R_SYM(i)     ((i) >> 32)
#define ELF64_R_TYPE(i)    ((i) & 0xffffFFFFL)
#define ELF64_R_INFO(s, t) (((s) << 32) + ((t) & 0xffffFFFFL))

// Program header table
typedef struct {
    Elf64_Word p_type;     // Type of segments
    Elf64_Word p_flags;    // Segments attributes
    Elf64_Off p_offset;    // Offset in file
    Elf64_Addr p_vaddr;    // Virtual address in memory
    Elf64_Addr p_paddr;    // Reserved
    Elf64_Xword p_filesz;  // Size of segment in file
    Elf64_Xword p_memsz;   // Size of segment in memory
    Elf64_Xword p_align;   // Alignment of segment
} Elf64_Phdr;

// Segment Types, p_type
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_LOOS    0x60000000
#define PT_HIOS    0x6FFFFFFF
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF

// Segments Attributes, p_flags
#define PF_X        0x1
#define PF_W        0x2
#define PF_R        0x3
#define PF_MASKOS   0x00FF0000
#define PF_MASKPROC 0xFF000000

// Dynamic Table
typedef struct {
    Elf64_Sxword d_tag;
    union {
        Elf64_Xword d_val;
        Elf64_Addr d_ptr;
    } d_un;
} Elf64_Dyn;

// Dynamic tables entries d_tag
#define DT_NULL   0  // End of the dynamic array
#define DT_NEEDED 1  // String table offset of the name of a needed lib
#define DT_PLTRELSZ                                                       \
    2  // Total size, in bytes, of the relocation entries associated with
       // procedure linkage table
#define DT_PLTGOT       3   // Address associated with the linkage table
#define DT_HASH         4   // Address of the symbol hash table
#define DT_STRTAB       5   // Address of the dynamic string table
#define DT_SYMTAB       6   // Address of the dynamic symbol table
#define DT_RELA         7   // Total size, in bytes, of the DT_RELA table
#define DT_RELASZ       8   // Size, in bytes, of each DT_RELA table
#define DT_RELAENT      9   // Size, in bytes, of each DT_RELA entry
#define DT_STRSZ        10  // Total size, in bytes, of the string table
#define DT_SYMENT       11  // Size, in byte,s of eah symbol table entry
#define DT_INIT         12  // Address of the initialization function
#define DT_FINI         13  // Address of the termination function
#define DT_SONAME       14  // String table offset of the name of this shared object
#define DT_RPATH        15  // String table offset of a shared lib search path
#define DT_SYMBOLIC     16  // See elf doc
#define DT_REL          17  // Address of a relocation table ith Elf64_rel entries
#define DT_RELSZ        18  // Total size, in bytes, of the DT_REL table
#define DT_RELENT       19  // Size, in bytes, of each DT_REL entry
#define DT_PLTREL       20  // Type of relocation entry used
#define DT_DEBUG        21  // Reserved for debugger use
#define DT_TEXTREL      22  // See elf doc
#define DT_JMPREL       23  // See elf doc
#define DT_BIND_NOW     24  // See elf doc
#define DT_INIT_ARRAY   25  // Pointer to an array of ptr to init functions
#define DT_FINI_ARRAY   26  // Pointer to an array of ptr to term functions
#define DT_INIT_ARRAYSZ 27  // Size of the array of the init functions
#define DT_FINI_ARRAYSZ 28  // Size of the array of the term functions
#define DT_LOOS         0x60000000  // Reserved for env specific use
#define DT_HIOS         0x6FFFFFFF
#define DT_LOPROC       0x70000000  // Reserved for processor specific use
#define DT_HIPROC       0x7FFFFFFF

char *get_elf_string(Elf64_Ehdr *header, u64 off);
Elf64_Shdr *get_section(Elf64_Ehdr *header, u64 idx);
Elf64_Phdr *get_segment(Elf64_Ehdr *header, u64 idx);

unsigned long elf64_hash(const unsigned char *name);

#endif  // ELF_H
