#include <nrf_gpio.h>
#include <nrf_delay.h>
#include <libraries/experimental_log/nrf_log.h>
#include <libraries/experimental_log/nrf_log_ctrl.h>
#include <libraries/experimental_log/nrf_log_default_backends.h>

int main(void) {
    nrf_gpio_pin_dir_set(17, NRF_GPIO_PIN_DIR_OUTPUT);

    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    while (true) {
        nrf_gpio_pin_toggle(17);
        nrf_delay_ms(1000);
        NRF_LOG_INFO("hello")
    }
}