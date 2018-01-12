//
// Created by Ethan Zhang on 10/01/2018.
//

#include "util.h"

// bit population - return number of on-bit
uint8_t bitpop(uint8_t bits) {
    uint8_t c;
    for (c = 0; bits; c++)
        bits &= bits - 1;
    return c;
}

uint8_t bitpop16(uint16_t bits) {
    uint8_t c;
    for (c = 0; bits; c++)
        bits &= bits - 1;
    return c;
}

uint8_t bitpop32(uint32_t bits) {
    uint8_t c;
    for (c = 0; bits; c++)
        bits &= bits - 1;
    return c;
}

uint8_t biton(uint8_t bits) {
    uint8_t n = 0;
    if (bits >> 4) {
        bits >>= 4;
        n += 4;
    }
    if (bits >> 2) {
        bits >>= 2;
        n += 2;
    }
    if (bits >> 1) {
        bits >>= 1;
        n += 1;
    }
    return n;
}

uint8_t biton16(uint16_t bits) {
    uint8_t n = 0;
    if (bits >> 8) {
        bits >>= 8;
        n += 8;
    }
    if (bits >> 4) {
        bits >>= 4;
        n += 4;
    }
    if (bits >> 2) {
        bits >>= 2;
        n += 2;
    }
    if (bits >> 1) {
        bits >>= 1;
        n += 1;
    }
    return n;
}

uint8_t biton32(uint32_t bits) {
    uint8_t n = 0;
    if (bits >> 16) {
        bits >>= 16;
        n += 16;
    }
    if (bits >> 8) {
        bits >>= 8;
        n += 8;
    }
    if (bits >> 4) {
        bits >>= 4;
        n += 4;
    }
    if (bits >> 2) {
        bits >>= 2;
        n += 2;
    }
    if (bits >> 1) {
        bits >>= 1;
        n += 1;
    }
    return n;
}


uint8_t bitrev(uint8_t bits) {
    bits = (bits & 0x0f) << 4 | (bits & 0xf0) >> 4;
    bits = (bits & 0b00110011) << 2 | (bits & 0b11001100) >> 2;
    bits = (bits & 0b01010101) << 1 | (bits & 0b10101010) >> 1;
    return bits;
}

uint16_t bitrev16(uint16_t bits) {
    bits = bitrev(bits & 0x00ff) << 8 | bitrev((bits & 0xff00) >> 8);
    return bits;
}

uint32_t bitrev32(uint32_t bits) {
    bits = (uint32_t) bitrev16(bits & 0x0000ffff) << 16 | bitrev16((bits & 0xffff0000) >> 16);
    return bits;
}