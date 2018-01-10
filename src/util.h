//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_UTIL_H
#define PW_HHKB_UTIL_H

#endif //PW_HHKB_UTIL_H

#include <stdint.h>

// convert to L string
#define LSTR(s) XLSTR(s)
#define XLSTR(s) L ## #s
// convert to string
#define STR(s) XSTR(s)
#define XSTR(s) #s


uint8_t bitpop(uint8_t bits);

uint8_t bitpop16(uint16_t bits);

uint8_t bitpop32(uint32_t bits);

uint8_t biton(uint8_t bits);

uint8_t biton16(uint16_t bits);

uint8_t biton32(uint32_t bits);

uint8_t bitrev(uint8_t bits);

uint16_t bitrev16(uint16_t bits);

uint32_t bitrev32(uint32_t bits);