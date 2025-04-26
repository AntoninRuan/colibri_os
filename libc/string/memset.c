#include <string.h>

void *memset(void *s, int c, size_t size) {
    unsigned char *dst = (unsigned char *)s;
    for (size_t i = 0; i < size; i++) {
        dst[i] = (unsigned char)c;
    }
    return s;
}
