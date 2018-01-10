//
// Created by Ethan Zhang on 10/01/2018.
//

#ifndef PW_HHKB_MAIN_H
#define PW_HHKB_MAIN_H

#endif //PW_HHKB_MAIN_H

#include "app_timer.h"
#include "config.h"
/**Buffer queue access macros */

/** Initialization of buffer list */
#define BUFFER_LIST_INIT() \
 do \
 { \
 buffer_list.rp = 0; \
 buffer_list.wp = 0; \
 buffer_list.count = 0; \
 } while (0)

/** Provide status of data list is full or not */
#define BUFFER_LIST_FULL() \
 ((MAX_BUFFER_ENTRIES == buffer_list.count - 1) ? true : false)

/** Provides status of buffer list is empty or not */
#define BUFFER_LIST_EMPTY() \
 ((0 == buffer_list.count) ? true : false)

#define BUFFER_ELEMENT_INIT(i) \
 do \
 { \
 buffer_list.buffer[(i)].p_data = NULL; \
 } while (0)

/** @} */

/** Abstracts buffer element */
typedef struct hid_key_buffer {
    uint8_t data_offset; /**< Max Data that can be buffered for all entries */
    uint8_t data_len; /**< Total length of data */
    uint8_t *p_data; /**< Scanned key pattern */
    ble_hids_t *p_instance; /**< Identifies peer and service instance */
} buffer_entry_t;STATIC_ASSERT(sizeof(buffer_entry_t) % 4 == 0);

/** Circular buffer list */
typedef struct {
    buffer_entry_t buffer[MAX_BUFFER_ENTRIES]; /**< Maximum number of entries that can enqueued in the list */
    uint8_t rp; /**< Index to the read location */
    uint8_t wp; /**< Index to write location */
    uint8_t count; /**< Number of elements in the list */
} buffer_list_t;STATIC_ASSERT(sizeof(buffer_list_t) % 4 == 0);

static ble_hids_t m_hids; /**< Structure used to identify the HID service. */
static ble_bas_t m_bas; /**< Structure used to identify the battery service. */
static bool m_in_boot_mode = false; /**< Current protocol mode. */
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID; /**< Handle of the current connection. */

static sensorsim_cfg_t m_battery_sim_cfg; /**< Battery Level sensor simulator configuration. */
static sensorsim_state_t m_battery_sim_state; /**< Battery Level sensor simulator state. */

APP_TIMER_DEF(m_battery_timer_id); /**< Battery timer. */

static pm_peer_id_t m_peer_id; /**< Device reference handle to the current bonded central. */
static bool m_caps_on = false; /**< Variable to indicate if Caps Lock is turned on. */

static pm_peer_id_t m_whitelist_peers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT]; /**< List of peers currently in the whitelist. */
static uint32_t m_whitelist_peer_cnt; /**< Number of peers currently in the whitelist. */
static bool m_is_wl_changed; /**< Indicates if the whitelist has been changed since last time it has been updated in the Peer Manager. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static uint8_t m_sample_key_press_scan_str[] = /**< Key pattern to be sent when the key press button has been pushed. */
        {
                0x0b, /* Key h */
                0x08, /* Key e */
                0x0f, /* Key l */
                0x0f, /* Key l */
                0x12, /* Key o */
                0x28 /* Key Return */
        };

static uint8_t m_caps_on_key_scan_str[] = /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit set. */
        {
                0x06, /* Key C */
                0x04, /* Key a */
                0x13, /* Key p */
                0x16, /* Key s */
                0x12, /* Key o */
                0x11, /* Key n */
        };

static uint8_t m_caps_off_key_scan_str[] = /**< Key pattern to be sent when the output report has been written with the CAPS LOCK bit cleared. */
        {
                0x06, /* Key C */
                0x04, /* Key a */
                0x13, /* Key p */
                0x16, /* Key s */
                0x12, /* Key o */
                0x09, /* Key f */
        };


static void on_hids_evt(ble_hids_t *p_hids, ble_hids_evt_t *p_evt);

static void peer_list_get(uint16_t *p_peers, __uint32_t *p_size);

/** List to enqueue not just data to be sent, but also related information like the handle, connection handle etc */
static buffer_list_t buffer_list;