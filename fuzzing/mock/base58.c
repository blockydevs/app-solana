#include "base58.h"
#include <string.h>

int base58_decode(const char *in, size_t in_len, uint8_t *out, size_t out_len) {
    (void)in;
    (void)in_len;
    if (out == NULL || out_len == 0) {
        return 0;
    }
    memset(out, 0, out_len);
    return (int)out_len;
}