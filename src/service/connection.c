#define NRF_LOG_MODULE_NAME CONNECTION

#include <nrf_log.h>

#include "src/service/connection.h"

#include <app_timer.h>
#include <common/ble_conn_params.h>

#include "src/config.h"

static void _errorHandler(uint32_t nrf_error);

void connection_init() {
    uint32_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = NULL;
    cp_init.error_handler = _errorHandler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

static void _errorHandler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}