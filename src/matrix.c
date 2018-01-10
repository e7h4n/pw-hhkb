#define NRF_LOG_MODULE_NAME "APP"

#include <nrf.h>
#include "nrf_log.h"

#include "matrix.h"
#include "util.h"

#define MATRIX_ROWS 8
#define MATRIX_COLS 8

#define ROW_PIN_START 2
#define COL_PIN_START 12
#define COL_CTRL_PIN 15
#define HYS_PIN 5
#define KEY_READ_PIN 7
#define MATRIX_POWER_PIN 8

//static const int STANDY_INPUT_GPIO = (0b111 << ROW_PIN_START)
//                                     | (0b1 << HYS_PIN)
//                                     | (0b111 << COL_PIN_START)
//                                     | (0b1 << COL_CTRL_PIN);
//
//static const int STANDY_OUTPUT_GPIO = (0b1 << KEY_READ_PIN)
//                                      | (0b1 << MATRIX_POWER_PIN);
//
//static const int SLEEP_GPIO = (0b111 << ROW_PIN_START)
//                              | (0b1 << HYS_PIN)
//                              | (0b111 << COL_PIN_START)
//                              | (0b1 << COL_CTRL_PIN)
//                              | (0b1 << KEY_READ_PIN)
//                              | (0b1 << MATRIX_POWER_PIN);

matrix_row_t currentState[MATRIX_ROWS] = {0};
matrix_row_t prevState[MATRIX_ROWS] = {0};
matrix_row_t zeroState[MATRIX_ROWS] = {0};

void matrix_print(void) {
    NRF_LOG_INFO("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        NRF_LOG_INFO("%02X: %08b%s\n", row, bitrev(matrix_get_row(row)));
    }
}

matrix_row_t matrix_get_row(uint8_t row) {
    return currentState[row];
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix_get_row(row) & (0b1 << col) >> col) == 1;
}

uint8_t matrix_scan(void) {
    return 0;
}

void matrix_init(void) {
}

uint8_t matrix_rows(void) {
    return MATRIX_COLS;
}

uint8_t matrix_cols(void) {
    return MATRIX_COLS;
}