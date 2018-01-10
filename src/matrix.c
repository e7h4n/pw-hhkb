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

static const int STANDBY_INPUT_GPIO = (0b111 << ROW_PIN_START)
                                      | (0b1 << HYS_PIN)
                                      | (0b111 << COL_PIN_START)
                                      | (0b1 << COL_CTRL_PIN);

static const int STANDBY_OUTPUT_GPIO = ~((0b1 << KEY_READ_PIN) | (0b1 << MATRIX_POWER_PIN));

static const int SLEEP_GPIO = ~(
        (0b111 << ROW_PIN_START)
        | (0b1 << HYS_PIN)
        | (0b111 << COL_PIN_START)
        | (0b1 << COL_CTRL_PIN)
        | (0b1 << KEY_READ_PIN)
        | (0b1 << MATRIX_POWER_PIN)
);

static const int PIN_PULLUP = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
                              | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                              | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                              | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                              | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

static matrix_row_t *matrix;
static matrix_row_t *matrix_prev;
static matrix_row_t _matrix0[MATRIX_ROWS];
static matrix_row_t _matrix1[MATRIX_ROWS];

matrix_row_t zeroState[MATRIX_ROWS] = {0};

void select(int row, int col);

void delayMicroseconds(int i);

void matrix_print(void) {
    NRF_LOG_INFO("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        NRF_LOG_INFO("%02X: %08b%s\n", row, bitrev(matrix_get_row(row)));
    }
}

matrix_row_t matrix_get_row(uint8_t row) {
    return matrix[row];
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return (matrix_get_row(row) & (0b1 << col) >> col) == 1;
}

void prevOn() {
    NRF_GPIO->OUT |= 1 << HYS_PIN;
}

void delayMicroseconds(int i) {

}

void select(int row, int col) {
    NRF_GPIO->OUT = NRF_GPIO->OUT | (row << ROW_PIN_START) | (col << COL_PIN_START) | (1 << COL_CTRL_PIN);
}

void enable() {
    NRF_GPIO->OUT &= ~(1 << COL_CTRL_PIN);
}

void disable() {
    NRF_GPIO->OUT = (NRF_GPIO->OUT & ~(0b111 << ROW_PIN_START) & ~(0b1 << HYS_PIN) & ~(0b111 << COL_PIN_START)) |
                    (0b1 << COL_CTRL_PIN);
}

bool powerState() {
    return bitRead(NRF_GPIO->DIR, MATRIX_POWER_PIN) == GPIO_PIN_CNF_DIR_Output;
}

void powerOff() {
    NRF_GPIO->DIR &= SLEEP_GPIO;

    for (int i = 0; i < 3; i++) {
        NRF_GPIO->PIN_CNF[i + ROW_PIN_START] = PIN_PULLUP;
        NRF_GPIO->PIN_CNF[i + COL_PIN_START] = PIN_PULLUP;
    }

    NRF_GPIO->PIN_CNF[HYS_PIN] = PIN_PULLUP;
    NRF_GPIO->PIN_CNF[COL_CTRL_PIN] = PIN_PULLUP;
    NRF_GPIO->PIN_CNF[POWER_ENABLED] = PIN_PULLUP;
    NRF_GPIO->PIN_CNF[KEY_READ_PIN] = PIN_PULLUP;
}

void powerOn() {
    NRF_GPIO->DIR = (NRF_GPIO->DIR | STANDBY_OUTPUT_GPIO) & STANDBY_INPUT_GPIO;
    NRF_GPIO->PIN_CNF[KEY_READ_PIN] = PIN_PULLUP;
    NRF_GPIO->OUT = bitSet(NRF_GPIO->OUT, MATRIX_POWER_PIN);
}

void matrix_init(void) {
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix0[i] = 0x00;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix1[i] = 0x00;
    matrix = _matrix0;
    matrix_prev = _matrix1;

    powerOff();
}

uint8_t matrix_rows(void) {
    return MATRIX_COLS;
}

uint8_t matrix_cols(void) {
    return MATRIX_COLS;
}

uint8_t matrix_scan(void) {
    uint8_t *tmp;

    tmp = matrix_prev;
    matrix_prev = matrix;
    matrix = tmp;

    if (!powerState()) {
        powerOn();
    }

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            select(row, col);

            delayMicroseconds(5);

            if (matrix_prev[row] & (0b1 << col)) {
                prevOn();
            }

            delayMicroseconds(10);

            enable();
            delayMicroseconds(5);

            int pressed = bitRead(NRF_GPIO->IN, KEY_READ_PIN);

            if (pressed == 0) {
#ifdef DEBUG
                Serial.printf("(%d, %d) ON\n", row, col);
#endif
                matrix[row] |= 0b1 << col;
            } else {
                matrix[row] &= ~(0b1 << col);
            }

            delayMicroseconds(5);
            disable();
            delayMicroseconds(75);
        }
    }
//
//    matrix_row_t matrix_change = 0;
//    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
//        matrix_change = matrix[r] ^ matrix_prev[r];
//
//        if (matrix_change) {
//            for (uint8_t c = 0; c < MATRIX_COLS; c++) {
//                if (matrix_change & (1 << c)) {
//                    keyevent_t e = (keyevent_t) {
//                            .key = (keypos_t) {.row = r, .col = c},
//                            .pressed = (matrix_row & ((matrix_row_t) 1 << c)),
//                            .time = (timer_read() | 1) /* time should not be 0 */
//                    };
//                    action_exec(e);
//                    hook_matrix_change(e);
//                    // record a processed key
//                    matrix_prev[r] ^= ((matrix_row_t) 1 << c);
//
//                    // This can miss stroke when scan matrix takes long like Topre
//                    // process a key per task call
//                    //goto MATRIX_LOOP_END;
//                }
//            }
//        }
//    }

    return 0;
}