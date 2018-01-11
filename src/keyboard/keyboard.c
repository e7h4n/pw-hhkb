//
// Created by 张宇辰 on 11/01/2018.
//

#include <src/util.h>
#include <keyboard/keyboard.h>
#include <keyboard/matrix.h>

static void onMatrixScan(matrix_row_t *matrix, matrix_row_t *matrix_prev);

void keyboard_init(void) {
    matrix_init(onMatrixScan);
}

static void onMatrixScan(matrix_row_t *matrix, matrix_row_t *matrix_prev) {
    matrix_row_t matrix_row = 0;
    matrix_row_t matrix_change = 0;
    uint8_t keyLen = 0;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row = matrix[row];
        matrix_change = matrix_row ^ matrix_prev[row];

        if (matrix_change) {
#ifdef DEBUG
            matrix_print();
#endif
            for (uint8_t col = 0; col < MATRIX_COLS && keyLen < 6; col++) {
                if (matrix_change & ((matrix_row_t) 1 << col)) {
                }
            }
        }
    }
}