#ifndef MATH_H
#define MATH_H

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

int ceildiv(int x, int y);
u8 ceillog2(u64 value);
void itoa(char *buf, unsigned int base, int d, bool caps);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // MATH_H
