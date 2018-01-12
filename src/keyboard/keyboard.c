//
// Created by 张宇辰 on 11/01/2018.
//

#include <src/util/util.h>
#include <keyboard/keyboard.h>
#include <keyboard/matrix.h>
#include <sched.h>

#define KEYBOARD_HID_KEYCODE_MODIFIER_MIN 0xE0
#define KEYBOARD_HID_KEYCODE_MODIFIER_MAX 0xE7
#define KEYBOARD_MAX_KEYS 6
#define KEYBOARD_UNUSED 0

/* KEYMAP KEY TO HID KEYCODE*/
uint8_t KEYMAP_NORMAL_MODE[MATRIX_ROWS][MATRIX_COLS] = {
        {0x1F/* 2 */, 0x14/* q */,   0x1A/* w */,      0x16/* s */,       0x04/* a */,            0x1D/* z */,       0x1B/* x */,     0x06/* c */},
        {0x20/* 3 */, 0x21/* 4 */,   0x15/* r */,      0x08/* e */,       0x07/* d */,            0x09/* f */,       0x19/* v */,     0x05/* b */},
        {0x22/* 5 */, 0x23/* 6 */,   0x1C/* y */,      0x17/* t */,       0x0A/* g */,            0x0B/* h */,       0x11/* n */, KEYBOARD_UNUSED},
        {0x1E/* 1 */, 0x29/* ESC */, 0x2B/* TAB */,    0xE0/* CONTROL */, 0xE1/* L-SHIFT */,      0xE2/* L-Alt */,   0xE3/* L-GUI */, 0x2C/* SPACE */},
        {0x24/* 7 */, 0x25/* 8 */,   0x18/* u */,      0x0C/* i */,       0x0E/* k */,            0x0D/* j */,       0x10/* m */, KEYBOARD_UNUSED},
        {0x31/* \ */, 0x35 /* ` */,  0x2A/* DELETE */, 0x28/* RETURN */, KEYBOARD_UNUSED/* Fn */, 0xE5/* R-SHIFT */, 0xE6/* R-Alt */, 0xE7/* R-GUI */},
        {0x26/* 9 */, 0x27/* 0 */,   0x12/* o */,      0x13/* p */,       0x33/* ; */,            0x0F/* l */,       0x36/* , */, KEYBOARD_UNUSED},
        {0x2D/* - */, 0x2E/* = */,   0x30/* ] */,      0x2F/* [ */,       0x34/* ' */,            0x38/* / */,       0x37/* . */, KEYBOARD_UNUSED}
};


/* KEYMAP KEY TO HID KEYCODE(Function Mode)*/
uint8_t KEYMAP_FN_MODE[MATRIX_ROWS][MATRIX_COLS] = {
        {0x3B/* F2 */, KEYBOARD_UNUSED, KEYBOARD_UNUSED,   0x80/* Vol Up */,  0x81/* Vol Dn */, KEYBOARD_UNUSED, KEYBOARD_UNUSED,        KEYBOARD_UNUSED},
        {0x3C/* F3 */,  0x3D/* F4 */,   KEYBOARD_UNUSED,   KEYBOARD_UNUSED,   0x7F/* Mute */,   KEYBOARD_UNUSED, KEYBOARD_UNUSED,        KEYBOARD_UNUSED},
        {0x3E/* F5 */,  0x3F/* F6 */,   KEYBOARD_UNUSED,   KEYBOARD_UNUSED,  KEYBOARD_UNUSED,   KEYBOARD_UNUSED, KEYBOARD_UNUSED,        KEYBOARD_UNUSED},
        {0x3A/* F1 */,  0x66/* Power */, 0x39/* CapsLck*/, KEYBOARD_UNUSED,  KEYBOARD_UNUSED,   KEYBOARD_UNUSED, KEYBOARD_UNUSED,        KEYBOARD_UNUSED},
        {0x40/* F7 */,  0x41/* F8 */,   KEYBOARD_UNUSED,   0x46/* PSc/SQq */, 0x4A/* Home */,   KEYBOARD_UNUSED, KEYBOARD_UNUSED,        KEYBOARD_UNUSED},
        {0x49/* Ins */, 0x4C/* (Del) */, 0x2A/* DELETE */, 0x28/* RETURN */, KEYBOARD_UNUSED,   KEYBOARD_UNUSED, KEYBOARD_UNUSED, 0x78/* Stop */},
        {0x42/* F9 */,  0x43/* F10 */,   0x47/* ScrLk */,  0x48/* Pus/Brk */, 0x50/* LeftArrow */,  0x4B/* PgUp */,      0x4D/* End */,  KEYBOARD_UNUSED},
        {0x44/* F11 */, 0x45/* F12 */,  KEYBOARD_UNUSED,   0x52/* UpArrow */, 0x4F/* RightArrow */, 0x51/* DownArrow */, 0x4E/* PgDn */, KEYBOARD_UNUSED}
};

static void onMatrixScan(matrix_row_t *matrix, matrix_row_t *matrix_prev);

static uint8_t getKeyCode(uint8_t row, uint8_t col, bool withFn);

static on_keyboard_event _onKeyboardEvent;
static uint8_t keyCodes[KEYBOARD_MAX_KEYS] = {0};
static uint8_t fnKeys[MATRIX_ROWS];

void keyboard_init(on_keyboard_event onKeyboardEvent) {
    _onKeyboardEvent = onKeyboardEvent;

    for (int i = 0; i < MATRIX_ROWS; i++) {
        fnKeys[i] = 0;
    }
    matrix_init(onMatrixScan);
}

void keyboard_deactive() {
    matrix_deactive();
}

void keyboard_active() {
    matrix_active();
}

static void onMatrixScan(matrix_row_t *matrix, matrix_row_t *matrix_prev) {
    uint8_t modifiers = 0x00;
    matrix_row_t matrix_row = 0;
    matrix_row_t matrix_change = 0;
    uint8_t keyLen = 0;
    uint8_t hidKeyCode = 0;

    bool withFn = (bool) bitRead(matrix[5], 4);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row = matrix[row];
        matrix_change = matrix_row ^ matrix_prev[row];

        if (!matrix_change && !matrix_row) {
            continue;
        }

        for (uint8_t col = 0; col < MATRIX_COLS && keyLen < 6; col++) {
            bool pressed = bitRead(matrix[row], col) > 0;

            if (withFn) {
                if (pressed) {
                    bitSet(fnKeys[row], col);
                } else {
                    bitClear(fnKeys[row], col);
                }
            }

            if (!pressed) {
                continue;
            }

            hidKeyCode = getKeyCode(row, col, bitRead(fnKeys[row], col) > 0);

            if (hidKeyCode >= KEYBOARD_HID_KEYCODE_MODIFIER_MIN &&
                hidKeyCode <= KEYBOARD_HID_KEYCODE_MODIFIER_MAX) {
                modifiers |= (1 << (hidKeyCode - KEYBOARD_HID_KEYCODE_MODIFIER_MIN));
            } else if (keyLen < KEYBOARD_MAX_KEYS) {
                if (hidKeyCode != KEYBOARD_UNUSED) {
                    keyCodes[keyLen++] = hidKeyCode;
                }
            }
        }
    }

    if (_onKeyboardEvent != NULL) {
        _onKeyboardEvent(modifiers,
                         (uint8_t) (keyLen > 0 ? keyCodes[0] : KEYBOARD_UNUSED),
                         (uint8_t) (keyLen > 1 ? keyCodes[1] : KEYBOARD_UNUSED),
                         (uint8_t) (keyLen > 2 ? keyCodes[2] : KEYBOARD_UNUSED),
                         (uint8_t) (keyLen > 3 ? keyCodes[3] : KEYBOARD_UNUSED),
                         (uint8_t) (keyLen > 4 ? keyCodes[4] : KEYBOARD_UNUSED),
                         (uint8_t) (keyLen > 5 ? keyCodes[5] : KEYBOARD_UNUSED)
        );
    }
}

static uint8_t getKeyCode(uint8_t row, uint8_t col, bool withFn) {
    if (!withFn) {
        return KEYMAP_NORMAL_MODE[row][col];
    }

    return KEYMAP_FN_MODE[row][col] != KEYBOARD_UNUSED ? KEYMAP_FN_MODE[row][col] : KEYMAP_NORMAL_MODE[row][col];
}
