//
// Created by Ethan Zhang on 12/01/2018.
//

#include "error.h"

#include <app_error.h>
#include <src/config.h>

void error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

void error_nrf_callback(uint16_t line_num, const uint8_t *p_file_name) {
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
