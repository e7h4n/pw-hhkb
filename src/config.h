//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_CONFIG_H
#define PW_HHKB_CONFIG_H

#endif //PW_HHKB_CONFIG_H

#if (NRF_SD_BLE_API_VERSION == 3)
#define NRF_BLE_MAX_MTU_SIZE GATT_MTU_SIZE_DEFAULT /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */
#endif

#define CENTRAL_LINK_COUNT 0 // Number of central links used by the application. When changing this number remember to adjust the RAM settings

// TODO: 搞懂这个干什么的
#define PERIPHERAL_LINK_COUNT 1 // Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define SHIFT_BUTTON_ID 1 // Button used as 'SHIFT' Key.

#define DEVICE_NAME "HHKB Pro2 BT"
#define MANUFACTURER_NAME "PFU & pw"

// TODO: 搞懂这个干什么的
#define APP_TIMER_PRESCALER 0 // Value of the RTC1 PRESCALER register.
#define APP_TIMER_OP_QUEUE_SIZE 4 // Size of timer operation queues.

// TODO: 实现正确的电量管理
#define BATTERY_LEVEL_MEAS_INTERVAL APP_TIMER_TICKS(2000, APP_TIMER_PRESCALER) // Battery level measurement interval (ticks).
#define MIN_BATTERY_LEVEL 81 // Minimum simulated battery level.
#define MAX_BATTERY_LEVEL 100 // Maximum simulated battery level.
#define BATTERY_LEVEL_INCREMENT 1 // Increment between each simulated battery level measurement.

// TODO: 使用正确的 PNP_ID
#define PNP_ID_VENDOR_ID_SOURCE 0x02 // Vendor ID Source.
#define PNP_ID_VENDOR_ID 0x1915 // Vendor ID.
#define PNP_ID_PRODUCT_ID 0xEEEE // Product ID.
#define PNP_ID_PRODUCT_VERSION 0x0001 // Product Version.

#define APP_ADV_FAST_INTERVAL 0x0028 // Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.).
#define APP_ADV_SLOW_INTERVAL 0x0C80 // Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds).
#define APP_ADV_FAST_TIMEOUT 30 // The duration of the fast advertising period (in seconds).
#define APP_ADV_SLOW_TIMEOUT 180 // The duration of the slow advertising period (in seconds).

// TODO: 搞懂这个干什么的 --- START
#define MIN_CONN_INTERVAL MSEC_TO_UNITS(7.5, UNIT_1_25_MS) // Minimum connection interval (7.5 ms)
#define MAX_CONN_INTERVAL MSEC_TO_UNITS(30, UNIT_1_25_MS) // Maximum connection interval (30 ms).
#define SLAVE_LATENCY 6 // Slave latency.
#define CONN_SUP_TIMEOUT MSEC_TO_UNITS(430, UNIT_10_MS) // Connection supervisory timeout (430 ms).

#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) // Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds).
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) // Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds).
#define MAX_CONN_PARAMS_UPDATE_COUNT 3 // Number of attempts before giving up the connection parameter negotiation.
// TODO: 搞懂这个干什么的 --- END

// TODO: 考虑安全性
#define SEC_PARAM_BOND 1 // Perform bonding.
#define SEC_PARAM_MITM 0 // Man In The Middle protection not required.
#define SEC_PARAM_LESC 0 // LE Secure Connections not enabled.
#define SEC_PARAM_KEYPRESS 0 // Keypress notifications not enabled.
#define SEC_PARAM_IO_CAPABILITIES BLE_GAP_IO_CAPS_NONE // No I/O capabilities.
#define SEC_PARAM_OOB 0 // Out Of Band data not available.
#define SEC_PARAM_MIN_KEY_SIZE 7 // Minimum encryption key size.
#define SEC_PARAM_MAX_KEY_SIZE 16 // Maximum encryption key size.

// TODO: 搞懂这个干什么的
#define OUTPUT_REPORT_INDEX 0 // Index of Output Report.
#define OUTPUT_REPORT_MAX_LEN 1 // Maximum length of Output Report.
#define INPUT_REPORT_KEYS_INDEX 0 // Index of Input Report.
#define OUTPUT_REPORT_BIT_MASK_CAPS_LOCK 0x02 // CAPS LOCK bit in Output Report (based on 'LED Page (0x08)' of the Universal Serial Bus HID Usage Tables).
#define INPUT_REP_REF_ID 0 // Id of reference to Keyboard Input Report.
#define OUTPUT_REP_REF_ID 0 // Id of reference to Keyboard Output Report.

// TODO: 搞懂这个干什么的
#define APP_FEATURE_NOT_SUPPORTED (BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2) // Reply when unsupported features are requested.

// TODO: 搞懂这个干什么的
#define MAX_BUFFER_ENTRIES 5 // Number of elements that can be enqueued

// TODO: 搞懂这个干什么的
#define BASE_USB_HID_SPEC_VERSION 0x0101 // Version number of base USB HID Specification implemented by this application.

// TODO: 搞懂这个干什么的
#define INPUT_REPORT_KEYS_MAX_LEN 8 // Maximum length of the Input Report characteristic.

// TODO: 搞懂这个干什么的
#define DEAD_BEEF 0xDEADBEEF // Value used as error code on stack dump, can be used to identify stack location on stack unwind.

// TODO: 搞懂这个干什么的
#define SCHED_MAX_EVENT_DATA_SIZE MAX(APP_TIMER_SCHED_EVT_SIZE, \
 BLE_STACK_HANDLER_SCHED_EVT_SIZE) // Maximum size of scheduler events.
#ifdef SVCALL_AS_NORMAL_FUNCTION
#define SCHED_QUEUE_SIZE 20 // Maximum number of events in the scheduler queue. More is needed in case of Serialization.
#else
#define SCHED_QUEUE_SIZE 10 // Maximum number of events in the scheduler queue.
#endif

// TODO: 搞懂这个干什么的
#define MODIFIER_KEY_POS 0 // Position of the modifier byte in the Input Report.
/*
 * This macro indicates the start position of the key scan code in a HID Report.
 * As per the document titled 'Device Class Definition for Human Interface Devices (HID) V1.11,
 * each report shall have one modifier byte followed by a reserved constant byte and then the key scan code
 */
#define SCAN_CODE_POS 2

#define MAX_KEYS_IN_ONE_REPORT (INPUT_REPORT_KEYS_MAX_LEN - SCAN_CODE_POS) // Maximum number of key presses that can be sent in one Input Report.