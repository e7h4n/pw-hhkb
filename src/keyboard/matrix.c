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

enum PHASE {
    END = 1, SELECT, PREV, READ, RESET
};

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
static enum PHASE scanStatus;

static on_matrix_scan onMatrixScan;

static void continueSelect();
static void onScanComplete();

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

static void onPhaseEnd() {
    row = 0;
    col = 0;

    uint8_t *tmp;

    tmp = matrix_prev;
    matrix_prev = matrix;
    matrix = tmp;

    if (!powerState()) {
        powerOn();
    }

    scanStatus = SELECT;
    // TODO: set timer
}

static void onPhaseSelect() {
    NRF_GPIO->OUT = NRF_GPIO->OUT | (row << ROW_PIN_START) | (col << COL_PIN_START) | (1 << COL_CTRL_PIN);
    scanStatus = PREV;
    // TODO: set timer 5us
}

static void onPhasePrev() {
    if (matrix_prev[row] & (0b1 << col)) {
        NRF_GPIO->OUT |= 1 << HYS_PIN;
    }

    scanStatus = READ;
    // TODO: set timer 10us
}

static void onPhaseRead() {
    NRF_GPIO->OUT &= ~(1 << COL_CTRL_PIN);

    bool pressed = !(bitRead(NRF_GPIO->IN, KEY_READ_PIN));

    if (pressed) {
        matrix[row] |= 0b1 << col;
    } else {
        matrix[row] &= ~(0b1 << col);
    }

    scanStatus = RESET;

    // TODO: set timer 5us
}

static void onPhaseReset() {
    // disable matrix
    NRF_GPIO->OUT = (NRF_GPIO->OUT & ~(0b111 << ROW_PIN_START) & ~(0b1 << HYS_PIN) & ~(0b111 << COL_PIN_START))
                    | (0b1 << COL_CTRL_PIN);

    // 还没扫到最后一个键
    if (row < MATRIX_ROWS - 1 && col < MATRIX_COLS - 1) {
        continueSelect();
        return;
    }

    // 扫描结束，触发按键事件，确定是否休眠
    onScanComplete();
}

static void continueSelect() {
    scanStatus = SELECT;

    col = (uint8_t) ((col + 1) % MATRIX_COLS);

    if (col == 0) {
        row += 1;
    }

    // TODO: set timer 80us
}

static void onScanComplete() {
    scanStatus = END;

    onMatrixScan(matrix, matrix_prev);

    // TODO: power saving

    // TODO: set timer
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
        case END:
            onPhaseEnd();
            break;

        case SELECT:
            onPhaseSelect();
            break;

        case PREV:
            onPhasePrev();
            break;

        case READ:
            onPhaseRead();
            break;

        case RESET:
            onPhaseReset();
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
    scanStatus = END;
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
