//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_MATRIX_H
#define PW_HHKB_MATRIX_H

#endif //PW_HHKB_MATRIX_H

#include <stdint.h>

typedef uint8_t matrix_row_t;

typedef void (*on_matrix_scan)(matrix_row_t *matrix, matrix_row_t *matrix_prev);

void matrix_init(on_matrix_scan onMatrixScan);

void matrix_scanStart();

void matrix_scanEnd();

void matrix_print();