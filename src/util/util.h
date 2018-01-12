//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_UTIL_H
#define PW_HHKB_UTIL_H

#endif //PW_HHKB_UTIL_H

#include <stdint.h>

#define UNUSED_METHOD __attribute__ ((unused))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01ul)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

uint8_t bitrev(uint8_t bits);