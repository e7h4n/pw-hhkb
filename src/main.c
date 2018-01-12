#include "main.h"

#define NRF_LOG_MODULE_NAME "APP"

#include "nrf_log_ctrl.h"

#include <app_scheduler.h>
#include <app_timer_appsh.h>
#include <ble_advertising/ble_advertising.h>
#include <softdevice_handler_appsh.h>
#include <src/util/buffer.h>
#include <src/service/ble.h>
#include <src/service/hid.h>
#include <src/service/connection.h>
#include <src/keyboard/keyboard.h>
#include <fstorage.h>
#include <src/service/advertising.h>
#include <src/util/util.h>

#include "service/device.h"
#include "service/battery.h"
#include "config.h"

static void sys_evt_dispatch(uint32_t sys_evt) {
    // Dispatch the system event to the fstorage module, where it will be
    // dispatched to the Flash Data Storage (FDS) module.
    fs_sys_event_handler(sys_evt);

    // Dispatch to the Advertising module last, since it will check if there are any
    // pending flash operations in fstorage. Let fstorage process system events first,
    // so that it can report correctly to the Advertising module.
    ble_advertising_on_sys_evt(sys_evt);
}

int main(void) {
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);
    ble_init();

    // Register with the SoftDevice handler module for BLE events.
    APP_ERROR_CHECK(softdevice_sys_evt_handler_set(sys_evt_dispatch));

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    advertising_init();
    device_init();
    battery_active();
    hid_init();
    connection_init();
    buffer_init();

    // TODO: start
    advertising_start();

    while (1) {
        app_sched_execute();

        if (NRF_LOG_PROCESS() == false) {
            APP_ERROR_CHECK(sd_app_evt_wait());
        }
    }
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
UNUSED_METHOD
static void sleep_mode_enter(void) {
    keyboard_deactive();
    battery_deactive();

    APP_ERROR_CHECK(sd_power_system_off());
}