#pragma once
#include <stddef.h>
#include <stdint.h>

int base58_decode(const char *in, size_t in_len, uint8_t *out, size_t out_len);