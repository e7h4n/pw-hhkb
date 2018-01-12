#define NRF_LOG_MODULE_NAME "MATRIX"

#include "src/keyboard/matrix.h"

#include <nrf.h>
#include <nrf_log.h>
#include <app_timer.h>

#include "src/config.h"
#include "src/util/util.h"

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

static void _continueSelect();

static void _onScanComplete();

static void _delayToLoop(uint32_t ticks);

static void _loop(void *context);

static void _clock(void *context);

static bool _powerState();

static void _powerOff();

static void _powerOn();

static void _onPhaseUp();

static void _onPhaseSelect();

static void _onPhasePrev();

static void _onPhaseRead();

static void _onPhaseReset();

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

static matrix_row_t *_matrix;
static matrix_row_t *_matrix_prev;
static matrix_row_t _matrix0[MATRIX_ROWS];
static matrix_row_t _matrix1[MATRIX_ROWS];
static uint8_t _row = 0;
static uint8_t _col = 0;

static enum PHASE _phase = STOPPED;

static uint8_t _idleSeconds = 0;

static matrix_scanEventHandler _scanEventHandler;

void matrix_init(matrix_scanEventHandler _onMatrixScan) {
    _onMatrixScan = _onMatrixScan;

    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix0[i] = 0x00;
    for (uint8_t i = 0; i < MATRIX_ROWS; i++) _matrix1[i] = 0x00;
    _matrix = _matrix0;
    _matrix_prev = _matrix1;

    _idleSeconds = 0;

    APP_ERROR_CHECK(app_timer_create(&matrixTimer, APP_TIMER_MODE_SINGLE_SHOT, _loop));
    APP_ERROR_CHECK(app_timer_create(&clockTimer, APP_TIMER_MODE_REPEATED, _clock));

    _powerOff();

    _phase = STOPPED;
}

void matrix_active() {
    if (_phase != STOPPED) {
        return;
    }

    _idleSeconds = 0;

    _phase = UP;

    _delayToLoop(DURATION_TO_START);

    app_timer_stop(clockTimer);
    APP_ERROR_CHECK(app_timer_start(clockTimer, ONE_SECOND, NULL));
}

void matrix_deactive() {
    if (_phase == STOPPED) {
        return;
    }

    APP_ERROR_CHECK(app_timer_stop(matrixTimer));
    APP_ERROR_CHECK(app_timer_stop(clockTimer));
    _powerOff();

    _phase = STOPPED;
}

UNUSED_METHOD
void matrix_print(void) {
    NRF_LOG_INFO("r/c 01234567\n");

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        NRF_LOG_INFO("%02X: %08b%s\n", row, bitrev(_matrix[row]));
    }
}

static void _continueSelect() {
    _col = (uint8_t) ((_col + 1) % MATRIX_COLS);

    if (_col == 0) {
        _row += 1;
    }

    _delayToLoop(DURATION_TO_UP);

    _phase = SELECT;
}

static void _onScanComplete() {
    _scanEventHandler(_matrix, _matrix_prev);

    if (_idleSeconds >= MAX_IDLE_TIME_SECONDS) {
        if (_powerState()) {
            _powerOff();
        }

        _delayToLoop(DURATION_TO_UP_SLOW);
    } else {
        _delayToLoop(DURATION_TO_UP);
    }

    _phase = UP;
}

static void _loop(void *context) {
    UNUSED_PARAMETER(context);

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
    switch (_phase) {
        case STOPPED:
            break;

        case UP:
            _onPhaseUp();
            break;

        case SELECT:
            _onPhaseSelect();
            break;

        case PREV:
            _onPhasePrev();
            break;

        case READ:
            _onPhaseRead();
            break;

        case RESET:
            _onPhaseReset();
            break;
    }
}

static void _clock(void *context) {
    UNUSED_PARAMETER(context);

    _idleSeconds++;
}

static void _delayToLoop(uint32_t ticks) {
    app_timer_stop(matrixTimer);

    APP_ERROR_CHECK(app_timer_start(matrixTimer, ticks, NULL));
}

static void _onPhaseReset() {
    // disable matrix
    NRF_GPIO->OUT = (NRF_GPIO->OUT & ~(0b111 << ROW_PIN_START) & ~(0b1 << HYS_PIN) & ~(0b111 << COL_PIN_START))
                    | (0b1 << COL_CTRL_PIN);

    // 还没扫到最后一个键
    if (_row < MATRIX_ROWS - 1 && _col < MATRIX_COLS - 1) {
        _continueSelect();
        return;
    }

    // 扫描结束，触发按键事件，确定是否休眠
    _onScanComplete();
}

static void _onPhaseRead() {
    NRF_GPIO->OUT &= ~(1 << COL_CTRL_PIN);

    bool pressed = !(bitRead(NRF_GPIO->IN, KEY_READ_PIN));

    if (pressed) {
        _matrix[_row] |= 0b1 << _col;
    } else {
        _matrix[_row] &= ~(0b1 << _col);
    }

    if (_col == MATRIX_COLS - 1 && (_matrix[_row] ^ _matrix_prev[_row])) {
        _idleSeconds = 0;
    }

    _delayToLoop(DURATION_TO_RESET);

    _phase = RESET;
}

static void _onPhasePrev() {
    if (_matrix_prev[_row] & (0b1 << _col)) {
        NRF_GPIO->OUT |= 1 << HYS_PIN;
    }

    _delayToLoop(DURATION_TO_READ);

    _phase = READ;
}

static bool _powerState() {
    return bitRead(NRF_GPIO->DIR, MATRIX_POWER_PIN) == GPIO_PIN_CNF_DIR_Output;
}

static void _powerOff() {
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

static void _powerOn() {
    NRF_GPIO->DIR = (NRF_GPIO->DIR | STANDBY_OUTPUT_GPIO) & STANDBY_INPUT_GPIO;
    NRF_GPIO->PIN_CNF[KEY_READ_PIN] = PIN_PULL_UP;
    NRF_GPIO->OUT = bitSet(NRF_GPIO->OUT, MATRIX_POWER_PIN);
}

static void _onPhaseUp() {
    _row = 0;
    _col = 0;

    uint8_t *tmp;

    tmp = _matrix_prev;
    _matrix_prev = _matrix;
    _matrix = tmp;

    if (_powerState()) {
        _delayToLoop(DURATION_TO_SELECT);
    } else {
        _powerOn();
        _delayToLoop(DURATION_TO_SELECT_FROM_SLEEP);
    }

    _phase = SELECT;
}

static void _onPhaseSelect() {
    NRF_GPIO->OUT = NRF_GPIO->OUT | (_row << ROW_PIN_START) | (_col << COL_PIN_START) | (1 << COL_CTRL_PIN);
    _delayToLoop(DURATION_TO_PREV);

    _phase = PREV;
}
