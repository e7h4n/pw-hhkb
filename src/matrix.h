//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_MATRIX_H
#define PW_HHKB_MATRIX_H

#endif //PW_HHKB_MATRIX_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t matrix_row_t;

uint8_t matrix_rows(void);

uint8_t matrix_cols(void);

void matrix_init(void);

uint8_t matrix_scan(void);

bool matrix_is_on(uint8_t row, uint8_t col);

matrix_row_t matrix_get_row(uint8_t row);

void matrix_print(void);