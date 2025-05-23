#ifndef TSS_H
#define TSS_H

#include <sys/cdefs.h>

void setup_tss();
void update_rsp0(u64 rsp0);

#endif  // TSS_H
