#include <stdbool.h>
#include <sys/cdefs.h>

u8 ceillog2(u64 value) {
    if (value == 0) return 0;
    if (value == 1) return 1;

    u8 result = 0;
    bool sup = false;
    while (value > 1) {
        if (value & 1) sup = true;

        value >>= 1;
        result++;
    }
    return result + (u8)sup;
}
