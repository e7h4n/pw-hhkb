//
// Created by 张宇辰 on 11/01/2018.
//

#ifndef PW_HHKB_KEYBOARD_H
#define PW_HHKB_KEYBOARD_H

#include <stdbool.h>
#include "stdint.h"

typedef void (*keyboard_eventHandler)(uint8_t modifiers, uint8_t *keyCodes, uint8_t keyCodeLen);

void keyboard_init(keyboard_eventHandler keyboardEventHandler);

void keyboard_deactive();

void keyboard_active();

#endif //PW_HHKB_KEYBOARD_H
