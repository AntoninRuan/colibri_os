#include <math.h>
#include <stdbool.h>

// Convert d to a string in base `base` and put it in buf
void itoa(char *buf, unsigned int base, int d, bool caps) {
    // *buf = 'x';
    char *p = buf;
    char *p1 = buf, *p2;
    unsigned int ud = d;

    if (base == 10 && d < 0) {
        *p = '-';
        p++;
        p1++;
        ud = -d;
    }

    char a = caps ? 'A' : 'a';

    do {
        int remainder = ud % base;

        *p = (remainder < 10) ? remainder + '0' : remainder + a - 10;
        p++;
    } while (ud /= base);

    // BUF should be a NULL terminated string
    *p = 0;

    // Reverse BUF
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}
