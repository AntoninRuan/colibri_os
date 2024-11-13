#include <stdbool.h>
#include <stdint.h>

uint8_t ceillog2(uint64_t value) {
    if (value == 0) return 0;
    if (value == 1) return 1;

    uint8_t result = 0;
    bool sup = false;
    while (value > 1) {
        if (value & 1) sup = true;

        value >>= 1;
        result ++;
    }
    return result + (uint8_t) sup;
}
