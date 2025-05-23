#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <elf.h>
#include <kernel/process.h>
#include <kernel/x86-64.h>

void init_scheduler();
proc_t *create_process(char *name, Elf64_Ehdr *elf, bool user_proc);
void schedule(int_frame_t *int_frame);

#endif  // SCHEDULER_H
