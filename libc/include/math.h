#ifndef MATH_H
#define MATH_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

int ceildiv(int x, int y);
uint8_t ceillog2(uint64_t value);
void itoa(char *buf, unsigned int base, int d, bool caps);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MATH_H
