#ifndef MATH_H
#define MATH_H

#include <stdbool.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int ceildiv(int x, int y);
void itoa(char *buf, unsigned int base, int d, bool caps);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MATH_H
