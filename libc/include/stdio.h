#ifndef STDIO_H
#define STDIO_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char* __restrict, ...);
int sprintf(char* __restrict, const char* __restrict, ...);
int vsprintf(char* restrict str, const char* restrict formats, va_list ap);
int putchar(uint8_t);

#ifdef __cplusplus
}
#endif

#endif // STDIO_H
