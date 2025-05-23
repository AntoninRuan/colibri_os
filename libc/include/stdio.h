#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stdint.h>
#include <sys/cdefs.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int printf(const char *__restrict, ...);
int sprintf(char *__restrict, const char *__restrict, ...);
int vsprintf(char *restrict str, const char *restrict formats, va_list ap);
int putchar(u8);

#ifdef __cplusplus
}
#endif

#endif  // STDIO_H
