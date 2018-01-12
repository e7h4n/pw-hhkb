
#ifndef PW_HHKB_ERROR_H
#define PW_HHKB_ERROR_H

#include <sched.h>

void error_handler(uint32_t nrf_error);

void error_nrf_callback(uint16_t line_num, const uint8_t *p_file_name);

#endif //PW_HHKB_ERROR_H