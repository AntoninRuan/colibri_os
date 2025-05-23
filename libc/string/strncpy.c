#include <string.h>

char *strncpy(char *restrict dst, const char *restrict src, size_t dsize) {
    size_t dlen = strnlen(src, dsize);
    memcpy(dst, src, dlen);
    memset(dst + dlen, 0, dsize - dlen);
    return dst;
}
