//
// Created by 张宇辰 on 11/01/2018.
//

#ifndef PW_HHKB_KEYBOARD_H
#define PW_HHKB_KEYBOARD_H

#endif //PW_HHKB_KEYBOARD_H

#include <stdbool.h>
#include "stdint.h"

typedef void (*on_keyboard_event)(uint8_t modifiers, uint8_t key0, uint8_t key1, uint8_t key2, uint8_t key3,
                                  uint8_t key4, uint8_t key5);

void keyboard_init(on_keyboard_event onKeyboardEvent);

void keyboard_deactive();

void keyboard_active();