//
// Created by Ethan Zhang on 12/01/2018.
//

#include <ble_services/ble_hids/ble_hids.h>
#include <nrf_log.h>
#include <src/util/error.h>
#include <src/config.h>
#include <app_error.h>
#include <src/keyboard/keyboard.h>
#include "hid.h"

static ble_hids_t m_hids; /**< Structure used to identify the HID service. */

static void onHidEvt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt);
static void onKeyboardEvent(uint8_t modifiers, uint8_t key0, uint8_t key1, uint8_t key2, uint8_t key3,
                            uint8_t key4, uint8_t key5);

ble_hids_t *hid_service() {
    return &m_hids;
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

    hids_init_obj.evt_handler = onHidEvt;
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

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);

    keyboard_init(onKeyboardEvent);
}

static void onKeyboardEvent(uint8_t modifiers, uint8_t key0, uint8_t key1, uint8_t key2, uint8_t key3,
                              uint8_t key4, uint8_t key5) {
    // TODO: send hid event
}

static void onHidRepCharWrite(ble_hids_evt_t *p_evt) {
    if (p_evt->params.char_write.char_id.rep_type == BLE_HIDS_REP_TYPE_OUTPUT) {
        uint32_t err_code;
        uint8_t report_val;
        uint8_t report_index = p_evt->params.char_write.char_id.rep_index;

        if (report_index == OUTPUT_REPORT_INDEX) {
            // This code assumes that the outptu report is one byte long. Hence the following
            // static assert is made.
            STATIC_ASSERT(OUTPUT_REPORT_MAX_LEN == 1);

            err_code = ble_hids_outp_rep_get(&m_hids,
                                             report_index,
                                             OUTPUT_REPORT_MAX_LEN,
                                             0,
                                             &report_val);
            APP_ERROR_CHECK(err_code);
        }
    }
}

static void onHidEvt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt) {
    switch (p_evt->evt_type) {
        case BLE_HIDS_EVT_BOOT_MODE_ENTERED:
            break;

        case BLE_HIDS_EVT_REPORT_MODE_ENTERED:
            break;

        case BLE_HIDS_EVT_REP_CHAR_WRITE:
            onHidRepCharWrite(p_evt);
            break;

        case BLE_HIDS_EVT_NOTIF_ENABLED:
            break;

        default:
            // No implementation needed.
            break;
    }
}