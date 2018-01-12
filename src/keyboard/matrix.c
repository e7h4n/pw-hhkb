#include "keyboard/matrix.h"

#define NRF_LOG_MODULE_NAME "APP"

#include <nrf.h>
#include <nrf_log.h>
#include <app_timer.h>

#include "src/util/util.h"
#include "config.h"

#define ROW_PIN_START 2
#define COL_PIN_START 12
#define COL_CTRL_PIN 15
#define HYS_PIN 5
#define KEY_READ_PIN 7
#define MATRIX_POWER_PIN 8
#define MAX_IDLE_TIME_SECONDS 600 // 10 mins
#define DURATION_TO_START APP_TIMER_TICKS(0, APP_TIMER_PRESCALER)
#define DURATION_TO_UP APP_TIMER_TICKS(80, APP_TIMER_PRESCALER)
#define DURATION_TO_UP_SLOW APP_TIMER_TICKS(19975, APP_TIMER_PRESCALER) // 50Hz, 20000 - 5 - 5 - 10 - 5
#define DURATION_TO_SELECT APP_TIMER_TICKS(0, APP_TIMER_PRESCALER)
#define DURATION_TO_SELECT_FROM_SLEEP APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)
#define DURATION_TO_PREV APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)
#define DURATION_TO_READ APP_TIMER_TICKS(10, APP_TIMER_PRESCALER)
#define DURATION_TO_RESET APP_TIMER_TICKS(5, APP_TIMER_PRESCALER)
#define ONE_SECOND APP_TIMER_TICKS(1 * 1000 * 1000, APP_TIMER_PRESCALER) // -O.O-

APP_TIMER_DEF(matrixTimer);
APP_TIMER_DEF(clockTimer);

enum PHASE {
    UP = 1, SELECT, PREV, READ, RESET, STOPPED
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
static enum PHASE phase = STOPPED;
static uint8_t idleSeconds = 0;

static on_matrix_scan onMatrixScan;

static void continueSelect();

static void onScanComplete();

static void delayToLoop(uint32_t ticks);

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

static void onPhaseUp() {
    row = 0;
    col = 0;

    uint8_t *tmp;

    tmp = matrix_prev;
    matrix_prev = matrix;
    matrix = tmp;

    if (powerState()) {
        delayToLoop(DURATION_TO_SELECT);
    } else {
        powerOn();
        delayToLoop(DURATION_TO_SELECT_FROM_SLEEP);
    }

    phase = SELECT;
}

static void onPhaseSelect() {
    NRF_GPIO->OUT = NRF_GPIO->OUT | (row << ROW_PIN_START) | (col << COL_PIN_START) | (1 << COL_CTRL_PIN);
    delayToLoop(DURATION_TO_PREV);

    phase = PREV;
}

static void onPhasePrev() {
    if (matrix_prev[row] & (0b1 << col)) {
        NRF_GPIO->OUT |= 1 << HYS_PIN;
    }

    delayToLoop(DURATION_TO_READ);

    phase = READ;
}

static void onPhaseRead() {
    NRF_GPIO->OUT &= ~(1 << COL_CTRL_PIN);

    bool pressed = !(bitRead(NRF_GPIO->IN, KEY_READ_PIN));

    if (pressed) {
        matrix[row] |= 0b1 << col;
    } else {
        matrix[row] &= ~(0b1 << col);
    }

    if (col == MATRIX_COLS - 1 && (matrix[row] ^ matrix_prev[row])) {
        idleSeconds = 0;
    }

    delayToLoop(DURATION_TO_RESET);

    phase = RESET;
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
    col = (uint8_t) ((col + 1) % MATRIX_COLS);

    if (col == 0) {
        row += 1;
    }

    delayToLoop(DURATION_TO_UP);

    phase = SELECT;
}

static void onScanComplete() {
    onMatrixScan(matrix, matrix_prev);

    if (idleSeconds >= MAX_IDLE_TIME_SECONDS) {
        if (powerState()) {
            powerOff();
        }

        delayToLoop(DURATION_TO_UP_SLOW);
    } else {
        delayToLoop(DURATION_TO_UP);
    }

    phase = UP;
}

UNUSED_METHOD
/**
 * 100us for a full loop
 *                                   80us
 *                  +-----------------------------------+
 *                  |                                   |
 *                  v                                   +
 *            +-->SELECT------>PREV------->READ------>RESET-----+
 *            |           5us        10us        5us            |
 *  SCAN_RATE |                                                 | 80us
 *            |                                                 |
 *            +-----------------------UP<-----------------------+
 *
 */
static void loop(void *context) {
    UNUSED_PARAMETER(context);

    switch (phase) {
        case STOPPED:
            break;

        case UP:
            onPhaseUp();
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

static void clock(void *context) {
    UNUSED_PARAMETER(context);

    idleSeconds++;
}

static void delayToLoop(uint32_t ticks) {
    app_timer_stop(matrixTimer);

    APP_ERROR_CHECK(app_timer_start(matrixTimer, ticks, NULL));
}

void matrix_init(on_matrix_scan _onMatrixScan) {
    onMatrixScan = _onMatrixScan;

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix0[i] = 0x00;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix1[i] = 0x00;
    matrix = _matrix0;
    matrix_prev = _matrix1;

    idleSeconds = 0;

    APP_ERROR_CHECK(app_timer_create(&matrixTimer, APP_TIMER_MODE_SINGLE_SHOT, loop));
    APP_ERROR_CHECK(app_timer_create(&clockTimer, APP_TIMER_MODE_REPEATED, clock));

    powerOff();

    phase = STOPPED;
}

void matrix_active() {
    if (phase != STOPPED) {
        return;
    }

    idleSeconds = 0;

    phase = UP;

    delayToLoop(DURATION_TO_START);

    app_timer_stop(clockTimer);
    APP_ERROR_CHECK(app_timer_start(clockTimer, ONE_SECOND, NULL));
}

void matrix_deactive() {
    if (phase == STOPPED) {
        return;
    }

    APP_ERROR_CHECK(app_timer_stop(matrixTimer));
    APP_ERROR_CHECK(app_timer_stop(clockTimer));
    powerOff();

    phase = STOPPED;
}

void matrix_print(void) {
    NRF_LOG_INFO("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        NRF_LOG_INFO("%02X: %08b%s\n", row, bitrev(matrix[row]));
    }
}
