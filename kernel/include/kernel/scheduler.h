#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <elf.h>
#include <kernel/process.h>
#include <kernel/x86-64.h>

void init_scheduler();
void run_proc(proc_t *p);
void schedule(int_frame_t *int_frame);

#endif  // SCHEDULER_H
