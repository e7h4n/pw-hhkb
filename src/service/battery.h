//
// Created by Ethan Zhang on 12/01/2018.
//

#ifndef PW_HHKB_BATTERY_H
#define PW_HHKB_BATTERY_H

#include <ble_services/ble_bas/ble_bas.h>
#include <sensorsim.h>

void battery_init();

void battery_active();

void battery_deactive();

ble_bas_t* battery_service();

#endif //PW_HHKB_BATTERY_H