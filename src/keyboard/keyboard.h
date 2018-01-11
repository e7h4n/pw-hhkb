//
// Created by 张宇辰 on 11/01/2018.
//

#ifndef PW_HHKB_KEYBOARD_H
#define PW_HHKB_KEYBOARD_H

#endif //PW_HHKB_KEYBOARD_H

#include <stdbool.h>
#include "stdint.h"

typedef struct {
    uint8_t col;
    uint8_t row;
} keyboard_pos_t;

typedef struct {
    keyboard_pos_t key;
    bool     pressed;
} keyboard_event_t;

void keyboard_init(void);