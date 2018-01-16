#define NRF_LOG_MODULE_NAME UTIL

#include <nrf_log.h>

#include "src/util/util.h"

uint8_t bitrev(uint8_t bits) {
    bits = (bits & 0x0f) << 4 | (bits & 0xf0) >> 4;
    bits = (bits & 0b00110011) << 2 | (bits & 0b11001100) >> 2;
    bits = (bits & 0b01010101) << 1 | (bits & 0b10101010) >> 1;
    return bits;
}