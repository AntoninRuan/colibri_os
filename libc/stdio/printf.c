#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool print(char *_, const char *data, size_t length) {
    const unsigned char *bytes = (const unsigned char *)data;
    for (size_t i = 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF) return false;
    }
    return true;
}

static inline int format(bool (*dump)(char *buf, const char *, size_t),
                         char *restrict dest, const char *restrict formats,
                         va_list parameters) {
    int written = 0;

    while (*formats != '\0') {
        size_t maxrem = INT_MAX - written;
        if (formats[0] != '%' || formats[1] == '%') {
            if (formats[0] == '%') formats++;
            size_t amount = 1;
            while (formats[amount] && formats[amount] != '%') amount++;
            if (maxrem < amount) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!dump(dest + written, formats, amount)) return -1;
            formats += amount;
            written += amount;
            continue;
        }

        const char *format_begun_at = formats++;
        size_t len;
        bool upper_case;
        switch (*formats) {
            case 'c':
                formats++;
                char c = (char)va_arg(parameters, int);
                if (!maxrem) {
                    // TODO: Set errno to EOVERFLOW
                    return -1;
                }
                if (!dump(dest + written, &c, sizeof(c))) return -1;
                written++;
                break;

            case 's':
                formats++;
                const char *str = va_arg(parameters, const char *);
                len = strlen(str);
                if (maxrem < len) {
                    // TODO: Set errno to EOVERFLOW
                    return -1;
                }
                if (!dump(dest + written, str, len)) return -1;
                written += len;
                break;

            case 'd':
            case 'i':
            case 'o':
            case 'x':
            case 'X':
                int base = 16;
                if (*formats == 'd' || *formats == 'i')
                    base = 10;
                else if (*formats == 'o')
                    base = 8;
                upper_case = *formats == 'X';
                formats++;
                unsigned int i = (unsigned int)va_arg(parameters, int);
                char buf[13] = {0};  // 12 is the maximum of digits in octal +1
                                     // for NULL terminated
                itoa(buf, base, i, upper_case);
                size_t len = strlen(buf);
                if (!dump(dest + written, buf, len)) return -1;
                written += len;
                break;

            default:
                formats = format_begun_at;
                len = strlen(formats);
                if (maxrem < len) {
                    // TODO: Set errno to EOVERFLOW
                    return -1;
                }
                if (!dump(dest + written, formats, len)) return -1;
                written += len;
                formats += len;
                break;
        }
    }
    return written;
}

int printf(const char *restrict formats, ...) {
    va_list parameters;
    va_start(parameters, formats);
    int rc = format(&print, NULL, formats, parameters);
    va_end(parameters);
    return rc;
}

static bool write_to_buf(char *restrict dst, const char *restrict src,
                         size_t len) {
    memcpy(dst, src, len);
    return true;
}

int vsprintf(char *restrict str, const char *restrict formats, va_list ap) {
    return format(&write_to_buf, str, formats, ap);
}

int sprintf(char *restrict str, const char *restrict formats, ...) {
    va_list parameters;
    va_start(parameters, formats);
    int rc = vsprintf(str, formats, parameters);
    va_end(parameters);
    return rc;
}
