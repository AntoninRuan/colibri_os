#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <sys/cdefs.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int sprintf(char* __restrict, const char* __restrict, ...);
int putchar(uint8_t);

#ifdef __cplusplus
}
#endif

#endif // STDIO_H
