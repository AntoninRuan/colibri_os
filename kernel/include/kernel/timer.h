#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <stdint.h>

void nanodelay(uint64_t);
void millidelay(uint64_t);
void arm_timer(uint64_t nano, bool periodic, bool local);

#endif  // TIMER_H
