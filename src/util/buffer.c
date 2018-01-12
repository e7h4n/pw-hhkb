//
// Created by Ethan Zhang on 12/01/2018.
//

#include <src/util/buffer.h>
#include <src/config.h>

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

/** List to enqueue not just data to be sent, but also related information like the handle, connection handle etc */
static buffer_list_t buffer_list;

void buffer_init() {
    uint32_t buffer_count;

    BUFFER_LIST_INIT();

    for (buffer_count = 0; buffer_count < MAX_BUFFER_ENTRIES; buffer_count++) {
        BUFFER_ELEMENT_INIT(buffer_count);
    }
}

uint32_t buffer_enqueue(ble_hids_t *p_hids,
                        uint8_t *p_key_pattern,
                        uint16_t pattern_len,
                        uint16_t offset) {
    buffer_entry_t *element;
    uint32_t err_code = NRF_SUCCESS;

    if (BUFFER_LIST_FULL()) {
        // Element cannot be buffered.
        err_code = NRF_ERROR_NO_MEM;
    } else {
        // Make entry of buffer element and copy data.
        element = &buffer_list.buffer[(buffer_list.wp)];
        element->p_instance = p_hids;
        element->p_data = p_key_pattern;
        element->data_offset = (uint8_t) offset;
        element->data_len = (uint8_t) pattern_len;

        buffer_list.count++;
        buffer_list.wp++;

        if (buffer_list.wp == MAX_BUFFER_ENTRIES) {
            buffer_list.wp = 0;
        }
    }

    return err_code;
}

uint32_t buffer_dequeue(bool tx_flag) {
    buffer_entry_t *p_element;
    uint32_t err_code = NRF_SUCCESS;
    uint16_t actual_len;

    if (BUFFER_LIST_EMPTY()) {
        err_code = NRF_ERROR_NOT_FOUND;
    } else {
        bool remove_element = true;

        p_element = &buffer_list.buffer[(buffer_list.rp)];

        if (tx_flag) {
//            err_code = send_key_scan_press_release(p_element->p_instance,
//                                                   p_element->p_data,
//                                                   p_element->data_len,
//                                                   p_element->data_offset,
//                                                   &actual_len);
            // An additional notification is needed for release of all keys, therefore check
            // is for actual_len <= element->data_len and not actual_len < element->data_len
            if ((err_code == BLE_ERROR_NO_TX_PACKETS) && (actual_len <= p_element->data_len)) {
                // Transmission could not be completed, do not remove the entry, adjust next data to
                // be transmitted
                p_element->data_offset = (uint8_t) actual_len;
                remove_element = false;
            }
        }

        if (remove_element) {
            BUFFER_ELEMENT_INIT(buffer_list.rp);

            buffer_list.rp++;
            buffer_list.count--;

            if (buffer_list.rp == MAX_BUFFER_ENTRIES) {
                buffer_list.rp = 0;
            }
        }
    }

    return err_code;
}