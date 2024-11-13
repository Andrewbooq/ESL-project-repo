#ifndef BLINKY_ASSERT_H__
#define BLINKY_ASSERT_H__

#include "nrf_delay.h"
#define UNUSED(expr) (void)(expr)

/* ASSERT */
#define BLINKY_LED_ASSERT_DELAY_MS  100

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    nrf_gpio_pin_write(BLINKY_LED_0_PIN, !BLINKY_LED_ACTIVE_STATE);
    nrf_gpio_pin_write(BLINKY_LED_1_PIN, !BLINKY_LED_ACTIVE_STATE);
    nrf_gpio_pin_write(BLINKY_LED_2_PIN, !BLINKY_LED_ACTIVE_STATE);
    nrf_gpio_pin_write(BLINKY_LED_3_PIN, !BLINKY_LED_ACTIVE_STATE);

    while (true)
    {
        nrf_gpio_pin_write(BLINKY_LED_0_PIN, BLINKY_LED_ACTIVE_STATE);
        nrf_delay_ms(BLINKY_LED_ASSERT_DELAY_MS);
        nrf_gpio_pin_write(BLINKY_LED_0_PIN, !BLINKY_LED_ACTIVE_STATE);
        
        nrf_gpio_pin_write(BLINKY_LED_1_PIN, BLINKY_LED_ACTIVE_STATE);
        nrf_delay_ms(BLINKY_LED_ASSERT_DELAY_MS);
        nrf_gpio_pin_write(BLINKY_LED_1_PIN, !BLINKY_LED_ACTIVE_STATE);
    }
}

#endif // BLINKY_ASSERT_H__
