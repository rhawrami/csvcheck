#ifndef CSVCHECK_CMR_SHARED
#define CSVCHECK_CMR_SHARED 777

#include <stdint.h>

// quote chracter
const uint8_t quote_char = '"';
// linefeed (LF)
const uint8_t linefeed_char = '\n';
// carriage return (CR)
const uint8_t carriagereturn_char = '\r';

// prefix_xor computes the prefix xor of a 64-bit bitmask.
inline uint64_t prefix_xor(uint64_t x) {
    uint64_t scratch = x << 1;
    x ^= scratch;
    scratch = x << 2;
    x ^= scratch;
    scratch = x << 4;
    x ^= scratch;
    scratch = x << 8;
    x ^= scratch;
    scratch = x << 16;
    x ^= scratch;
    scratch = x << 32;
    x ^= scratch;

    return x;
}

#endif