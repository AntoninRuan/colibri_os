#ifndef CPU_H
#define CPU_H

// Struct use to read the value of general purposer register after a pushal
struct registers_t {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecpx;
    uint32_t eax;
};

#endif // CPU_H
