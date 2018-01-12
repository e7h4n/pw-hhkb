#ifndef PW_HHKB_BUFFER_H
#define PW_HHKB_BUFFER_H

#include <sched.h>

#include <ble_services/ble_hids/ble_hids.h>
void buffer_init();

uint32_t buffer_enqueue(ble_hids_t *p_hids,
                        uint8_t *p_key_pattern,
                        uint16_t pattern_len,
                        uint16_t offset);

uint32_t buffer_dequeue(bool tx_flag);

#endif //PW_HHKB_BUFFER_H