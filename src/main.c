#include <nrf_gpio.h>
#include <nrf_delay.h>

int main(void) {
    nrf_gpio_pin_dir_set(17, NRF_GPIO_PIN_DIR_OUTPUT);

    while (true) {
        nrf_gpio_pin_toggle(17);
        nrf_delay_ms(1000);
    }
}