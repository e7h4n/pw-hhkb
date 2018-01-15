#define NRF_LOG_MODULE_NAME "APP"

#include <app_scheduler.h>
#include <app_timer_appsh.h>
#include <ble_advertising/ble_advertising.h>
#include <fstorage.h>
#include <nrf_log_ctrl.h>
#include <softdevice_handler_appsh.h>

#include "src/config.h"
#include "src/keyboard/keyboard.h"
#include "src/service/advertising.h"
#include "src/service/battery.h"
#include "src/service/ble.h"
#include "src/service/hid.h"
#include "src/util/util.h"

static void _timerInit();

static void _schedInit();

static void _sysEvtDispatch(uint32_t sys_evt);

int main(void) {
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));

    _timerInit();
    _schedInit();

    ble_init();
    battery_init();
    hid_init();

    battery_active();
    keyboard_active();
    advertising_active();

    APP_ERROR_CHECK(softdevice_sys_evt_handler_set(_sysEvtDispatch));

    while (true) {
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false) {
            APP_ERROR_CHECK(sd_app_evt_wait());
        }
    }
}

void _schedInit() { APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); }

void _timerInit() { APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true); }

UNUSED_METHOD
static void _sleep(void) {
    keyboard_deactive();
    battery_deactive();

    APP_ERROR_CHECK(sd_power_system_off());
}

static void _sysEvtDispatch(uint32_t sys_evt) {
    fs_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}