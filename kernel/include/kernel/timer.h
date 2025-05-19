#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

void nanodelay(u64);
void millidelay(u64);
void arm_timer(u64 nano, bool periodic, bool local);

#endif  // TIMER_H
