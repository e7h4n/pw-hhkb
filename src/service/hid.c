#define NRF_LOG_MODULE_NAME HID

#include "src/service/hid.h"

#include <app_error.h>
#include <nrf_log.h>

#include "src/config.h"
#include "src/keyboard/keyboard.h"
#include "src/util/error.h"

#define INPUT_REPORT_KEYS_MAX_LEN 8

typedef struct {
    uint8_t keyPattern[INPUT_REPORT_KEYS_MAX_LEN];
    uint8_t keyPatternLen;
} key_pattern_t;

static void _onHidEvt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt);

static void _onHidRepCharWrite(ble_hids_evt_t *p_evt);

static void _onKeyboardEvent(uint8_t modifiers, uint8_t *keyCodes, uint8_t keyCodeLen);

static uint32_t _sendKeyPattern(key_pattern_t *keyPattern);

static uint32_t _bufferDequeue();

static void _bufferClear();

static ble_hids_t hidService;
static key_pattern_t keyPatternBuffer[MAX_BUFFER_ENTRIES][8] = {0};
static uint8_t keyPatternBufferSize;
static uint8_t keyPatternReadIndex;
static uint8_t keyPatternWriteIndex;
static bool isTransmitting = false;
static bool isBootMode = false;

ble_hids_t *hid_service() {
    return &hidService;
}

void hid_init() {
    uint32_t err_code;
    ble_hids_init_t hids_init_obj;
    ble_hids_inp_rep_init_t input_report_array[1];
    ble_hids_inp_rep_init_t *p_input_report;
    ble_hids_outp_rep_init_t output_report_array[1];
    ble_hids_outp_rep_init_t *p_output_report;
    uint8_t hid_info_flags;

    memset((void *) input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *) output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

    static uint8_t report_map_data[] = {
            0x05, 0x01,       // Usage Page (Generic Desktop)
            0x09, 0x06,       // Usage (Keyboard)
            0xA1, 0x01,       // Collection (Application)
            0x05, 0x07,       // Usage Page (Key Codes)
            0x19, 0xe0,       // Usage Minimum (224)
            0x29, 0xe7,       // Usage Maximum (231)
            0x15, 0x00,       // Logical Minimum (0)
            0x25, 0x01,       // Logical Maximum (1)
            0x75, 0x01,       // Report Size (1)
            0x95, 0x08,       // Report Count (8)
            0x81, 0x02,       // Input (Data, Variable, Absolute)

            0x95, 0x01,       // Report Count (1)
            0x75, 0x08,       // Report Size (8)
            0x81, 0x01,       // Input (Constant) reserved byte(1)

            0x95, 0x05,       // Report Count (5)
            0x75, 0x01,       // Report Size (1)
            0x05, 0x08,       // Usage Page (Page# for LEDs)
            0x19, 0x01,       // Usage Minimum (1)
            0x29, 0x05,       // Usage Maximum (5)
            0x91, 0x02,       // Output (Data, Variable, Absolute), Led report
            0x95, 0x01,       // Report Count (1)
            0x75, 0x03,       // Report Size (3)
            0x91, 0x01,       // Output (Data, Variable, Absolute), Led report padding

            0x95, 0x06,       // Report Count (6)
            0x75, 0x08,       // Report Size (8)
            0x15, 0x00,       // Logical Minimum (0)
            0x25, 0x65,       // Logical Maximum (101)
            0x05, 0x07,       // Usage Page (Key codes)
            0x19, 0x00,       // Usage Minimum (0)
            0x29, 0x65,       // Usage Maximum (101)
            0x81, 0x00,       // Input (Data, Array) Key array(6 bytes)

            0x09, 0x05,       // Usage (Vendor Defined)
            0x15, 0x00,       // Logical Minimum (0)
            0x26, 0xFF, 0x00, // Logical Maximum (255)
            0x75, 0x08,       // Report Count (2)
            0x95, 0x02,       // Report Size (8 bit)
            0xB1, 0x02,       // Feature (Data, Variable, Absolute)

            0xC0              // End Collection (Application)
    };

    // Initialize HID Service
    p_input_report = &input_report_array[INPUT_REPORT_KEYS_INDEX];
    p_input_report->max_len = INPUT_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id = INPUT_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    p_output_report = &output_report_array[OUTPUT_REPORT_INDEX];
    p_output_report->max_len = OUTPUT_REPORT_MAX_LEN;
    p_output_report->rep_ref.report_id = OUTPUT_REP_REF_ID;
    p_output_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_OUTPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_output_report->security_mode.write_perm);

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler = _onHidEvt;
    hids_init_obj.error_handler = error_handler;
    hids_init_obj.is_kb = true;
    hids_init_obj.is_mouse = false;
    hids_init_obj.inp_rep_count = 1;
    hids_init_obj.p_inp_rep_array = input_report_array;
    hids_init_obj.outp_rep_count = 1;
    hids_init_obj.p_outp_rep_array = output_report_array;
    hids_init_obj.feature_rep_count = 0;
    hids_init_obj.p_feature_rep_array = NULL;
    hids_init_obj.rep_map.data_len = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data = report_map_data;
    hids_init_obj.hid_information.bcd_hid = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags = hid_info_flags;
    hids_init_obj.included_services_count = 0;
    hids_init_obj.p_included_services_array = NULL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(
            &hids_init_obj.security_mode_boot_kb_inp_rep.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_inp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_boot_kb_inp_rep.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_boot_kb_outp_rep.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

    err_code = ble_hids_init(&hidService, &hids_init_obj);
    APP_ERROR_CHECK(err_code);

    keyboard_init(_onKeyboardEvent);
}

void hid_onBleEvent(ble_evt_t *p_ble_evt) {
    switch (p_ble_evt->header.evt_id) {
        case BLE_EVT_TX_COMPLETE:
            if (keyPatternBufferSize == 0) {
                isTransmitting = false;
            } else {
                _bufferDequeue();
            }
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected\r\n");
            isTransmitting = false;
            _bufferClear();
            break;
        default:
            break;
    }
}

static void _onKeyboardEvent(uint8_t modifiers, uint8_t *keyCodes, uint8_t keyCodeLen) {
    if (keyPatternBufferSize == MAX_BUFFER_ENTRIES) {
        // buffer is full, ignore input
        return;
    }

    keyPatternBuffer[keyPatternWriteIndex]->keyPattern[0] = modifiers;
    keyPatternBuffer[keyPatternWriteIndex]->keyPatternLen = (uint8_t) (keyCodeLen + 2);

    for (uint8_t i = 0; i < keyCodeLen; i++) {
        keyPatternBuffer[keyPatternWriteIndex]->keyPattern[i + 2] = keyCodes[i];
    }

    keyPatternBufferSize++;
    keyPatternWriteIndex++;

    if (keyPatternWriteIndex == MAX_BUFFER_ENTRIES) {
        keyPatternWriteIndex = 0;
    }

    if (!isTransmitting) {
        _bufferDequeue();
    }
}

static void _onHidEvt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt) {
    UNUSED_PARAMETER(p_hids);

    switch (p_evt->evt_type) {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            isBootMode = true;
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            isBootMode = false;
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            _onHidRepCharWrite(p_evt);
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            break;

        default:
            break;
    }
}

static void _onHidRepCharWrite(ble_hids_evt_t *p_evt) {
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        uint32_t err_code;
        uint8_t report_val;
        uint8_t report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX) {
            // This code assumes that the outptu report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&hidService,
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             &report_val);
            APP_ERROR_CHECK(err_code);
        }
    }
}

static uint32_t _sendKeyPattern(key_pattern_t *keyPattern) {
    uint32_t errorCode;

    if (!isBootMode) {
        errorCode = ble_hids_inp_rep_send(&hidService, INPUT_REPORT_KEYS_INDEX, keyPattern->keyPatternLen, keyPattern->keyPattern);
    } else {
        errorCode = ble_hids_boot_kb_inp_rep_send(&hidService, keyPattern->keyPatternLen, keyPattern->keyPattern);
    }

    return errorCode;
}

static uint32_t _bufferDequeue() {
    uint32_t err_code;

    if (keyPatternBufferSize == 0) {
        err_code = NRF_ERROR_NOT_FOUND;
    } else {
        bool remove_element = true;

        err_code = _sendKeyPattern(keyPatternBuffer[keyPatternReadIndex]);

        // An additional notification is needed for release of all keys, therefore check
        // is for actual_len <= element->data_len and not actual_len < element->data_len
        if (err_code == BLE_ERROR_NO_TX_PACKETS) {
            remove_element = false;
        } else {
            isTransmitting = true;
        }

        if (remove_element) {
            keyPatternReadIndex++;
            keyPatternBufferSize--;

            if (keyPatternReadIndex == MAX_BUFFER_ENTRIES) {
                keyPatternReadIndex = 0;
            }
        }
    }

    return err_code;
}

static void _bufferClear() {
    while (keyPatternBufferSize > 0) {
        keyPatternReadIndex++;
        keyPatternBufferSize--;

        if (keyPatternReadIndex == MAX_BUFFER_ENTRIES) {
            keyPatternReadIndex = 0;
        }
    }
}
