#ifndef TSS_H
#define TSS_H

#include <sys/cdefs.h>

void setup_tss();
u64 get_rsp0();
void update_rsp0(u64 rsp0);

#endif  // TSS_H
