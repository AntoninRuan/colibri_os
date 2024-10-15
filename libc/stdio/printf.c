#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static bool print(const char* data, size_t length) {
    const unsigned char* bytes = (const unsigned char*) data;
    for (size_t i= 0; i < length; i++) {
        if (putchar(bytes[i]) == EOF) return false;
    }
    return true;
}

int printf(const char* restrict format, ...) {
    va_list parameters;
    va_start(parameters, format);

    int written = 0;

    while (*format != '\0') {
        size_t maxrem = INT_MAX - written;
        if (format[0] != '%' || format[1] == '%') {
            if (format[0] == '%') format ++;
            size_t amount = 1;
            while (format[amount] && format[amount] != '%') amount ++;
            if (maxrem < amount) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!print(format, amount)) return -1;
            format += amount;
            written += amount;
            continue;
        }

        const char* format_begun_at = format++;
        size_t len;
        bool upper_case;
        switch (*format) {
        case 'c':
            format++;
            char c = (char) va_arg(parameters, int);
            if (!maxrem) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!print(&c, sizeof(c))) return -1;
            written ++;
            break;

        case 's':
            format ++;
            const char* str = va_arg(parameters, const char*);
            len = strlen(str);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!print(str, len)) return -1;
            written += len;
            break;

        case 'd':
        case 'i':
        case 'o':
        case 'x':
        case 'X':
            int base = 16;
            if (*format == 'd' || *format == 'i') base = 10;
            else if (*format == 'o') base = 8;
            upper_case = *format == 'X';
            format++;
            unsigned int i = (unsigned int) va_arg(parameters, int);
            char buf[13] = {0}; // 12 is the maximum of digits in octal +1 for NULL terminated
            itoa(buf, base, i, upper_case);
            if (!print(buf, strlen(buf))) return -1;
            break;

        default:
            format = format_begun_at;
            len = strlen(format);
            if (maxrem < len) {
                // TODO: Set errno to EOVERFLOW
                return -1;
            }
            if (!print(format, len)) return -1;
            written += len;
            format += len;
            break;
        }
    }

    va_end(parameters);
    return written;
}
