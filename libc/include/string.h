#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void *, const void *, size_t);
void *memcpy(void *__restrict, const void *__restrict, size_t);
void *memmove(void *, const void *, size_t);
void *memset(void *s, int c, size_t size);
char *strncpy(char *restrict dst, const char *restrict src, size_t dsize);
size_t strlen(const char *);
size_t strnlen(const char *, size_t maxlen);

#ifdef __cplusplus
}
#endif

#endif  // STRING_H
