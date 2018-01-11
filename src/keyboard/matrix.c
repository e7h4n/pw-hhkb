#include "keyboard/matrix.h"

#define NRF_LOG_MODULE_NAME "APP"

#include <nrf.h>
#include <nrf_log.h>

#include "util.h"
#include "config.h"

#define ROW_PIN_START 2
#define COL_PIN_START 12
#define COL_CTRL_PIN 15
#define HYS_PIN 5
#define KEY_READ_PIN 7
#define MATRIX_POWER_PIN 8

#define SCAN_PHASE_SELECT 0
#define SCAN_PHASE_PREV 1
#define SCAN_PHASE_READ 2
#define SCAN_PHASE_RESET 3
#define SCAN_PHASE_END 4

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

static const int PIN_PULL_UP = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos)
                               | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                               | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                               | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                               | (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

static matrix_row_t *matrix;
static matrix_row_t *matrix_prev;
static matrix_row_t _matrix0[MATRIX_ROWS];
static matrix_row_t _matrix1[MATRIX_ROWS];

static uint8_t row = 0;
static uint8_t col = 0;
static int scanStatus = SCAN_PHASE_END;

static on_matrix_scan onMatrixScan;

static void enableHysControl() {
    NRF_GPIO->OUT |= 1 << HYS_PIN;
}

static void select(int row, int col) {
    NRF_GPIO->OUT = NRF_GPIO->OUT | (row << ROW_PIN_START) | (col << COL_PIN_START) | (1 << COL_CTRL_PIN);
}

static void enable() {
    NRF_GPIO->OUT &= ~(1 << COL_CTRL_PIN);
}

static void disable() {
    NRF_GPIO->OUT = (NRF_GPIO->OUT & ~(0b111 << ROW_PIN_START) & ~(0b1 << HYS_PIN) & ~(0b111 << COL_PIN_START)) |
                    (0b1 << COL_CTRL_PIN);
}

UNUSED
static bool powerState() {
    return bitRead(NRF_GPIO->DIR, MATRIX_POWER_PIN) == GPIO_PIN_CNF_DIR_Output;
}

static void powerOff() {
    NRF_GPIO->DIR &= SLEEP_GPIO;

    for (int i = 0; i < 3; i++) {
        NRF_GPIO->PIN_CNF[i + ROW_PIN_START] = PIN_PULL_UP;
        NRF_GPIO->PIN_CNF[i + COL_PIN_START] = PIN_PULL_UP;
    }

    NRF_GPIO->PIN_CNF[HYS_PIN] = PIN_PULL_UP;
    NRF_GPIO->PIN_CNF[COL_CTRL_PIN] = PIN_PULL_UP;
    NRF_GPIO->PIN_CNF[POWER_ENABLED] = PIN_PULL_UP;
    NRF_GPIO->PIN_CNF[KEY_READ_PIN] = PIN_PULL_UP;
}

static void powerOn() {
    NRF_GPIO->DIR = (NRF_GPIO->DIR | STANDBY_OUTPUT_GPIO) & STANDBY_INPUT_GPIO;
    NRF_GPIO->PIN_CNF[KEY_READ_PIN] = PIN_PULL_UP;
    NRF_GPIO->OUT = bitSet(NRF_GPIO->OUT, MATRIX_POWER_PIN);
}

/**
 * 完整扫描一个键需要 100us
 *                                   80us
 *                  +-----------------------------------+
 *                  |                                   |
 *                  v                                   +
 *            +-->SELECT------>PREV------->READ------>RESET-----+
 *            |           5us        10us        5us            |
 *  SCAN_RATE |                                                 | 80us
 *            |                                                 |
 *            +----------------------END<-----------------------+
 *
 */
UNUSED
static void loop() {
    switch (scanStatus) {
        case SCAN_PHASE_END:
            row = 0;
            col = 0;

            uint8_t *tmp;

            tmp = matrix_prev;
            matrix_prev = matrix;
            matrix = tmp;

            powerOn();

            scanStatus = SCAN_PHASE_SELECT;
            // TODO: set timer
            break;

        case SCAN_PHASE_SELECT:
            select(row, col);

            scanStatus = SCAN_PHASE_PREV;
            // TODO: set timer 5us
            break;

        case SCAN_PHASE_PREV:
            if (matrix_prev[row] & (0b1 << col)) {
                enableHysControl();
            }

            scanStatus = SCAN_PHASE_READ;
            // TODO: set timer 10us
            break;

        case SCAN_PHASE_READ:
            enable();

            bool pressed = !(bitRead(NRF_GPIO->IN, KEY_READ_PIN));

            if (pressed) {
                matrix[row] |= 0b1 << col;
            } else {
                matrix[row] &= ~(0b1 << col);
            }

            scanStatus = SCAN_PHASE_RESET;

            // TODO: set timer 5us
            break;

        case SCAN_PHASE_RESET:
            disable();

            // 还没扫到最后一个键
            if (row < MATRIX_ROWS - 1 && col < MATRIX_COLS - 1) {
                scanStatus = SCAN_PHASE_SELECT;

                col = (uint8_t) ((col + 1) % MATRIX_COLS);

                if (col == 0) {
                    row += 1;
                }

                // TODO: set timer 80us
                break;
            }

            // 扫描结束，触发按键事件，确定是否休眠

            scanStatus = SCAN_PHASE_END;

            onMatrixScan(matrix, matrix_prev);

            // TODO: power saving

            // TODO: set timer
            break;
    }
}

void matrix_init(on_matrix_scan _onMatrixScan) {
    onMatrixScan = _onMatrixScan;

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix0[i] = 0x00;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix1[i] = 0x00;
    matrix = _matrix0;
    matrix_prev = _matrix1;

    powerOff();
}

void matrix_scanStart() {
    scanStatus = SCAN_PHASE_END;
    // TODO: set timer
}

void matrix_scanEnd() {
    // TODO: unset timer
    // TODO: 确定是否需要休眠
}

void matrix_print(void) {
    NRF_LOG_INFO("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        NRF_LOG_INFO("%02X: %08b%s\n", row, bitrev(matrix[row]));
    }
}
