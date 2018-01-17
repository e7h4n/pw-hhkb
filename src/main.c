#include <nrf_gpio.h>
#include <nrf_delay.h>
#include <SEGGER_RTT.h>

int main(void) {
    uint8_t count = 0;
    nrf_gpio_pin_dir_set(19, NRF_GPIO_PIN_DIR_OUTPUT);

    while (true) {
        nrf_gpio_pin_toggle(19);
        nrf_delay_ms(2000);
        SEGGER_RTT_printf(0, "[%d]Hello PW-HHKB!\n", count);

        count = (count + 1) % 8;
    }
}