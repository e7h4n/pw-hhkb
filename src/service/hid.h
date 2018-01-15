//
// Created by Ethan Zhang on 12/01/2018.
//

#ifndef PW_HHKB_HID_H
#define PW_HHKB_HID_H

#include <ble_hids.h>

void hid_init();

ble_hids_t* hid_service();

void hid_onBleEvent(ble_evt_t *p_ble_evt);

#endif //PW_HHKB_HID_H