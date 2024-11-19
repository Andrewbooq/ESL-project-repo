#include "nrf_delay.h"
#include "blinky_led.h"

#ifdef BLINKY_LED_PWM_CONTROL
#include "blinky_led_pwm.h"
#endif

#define BLINKY_LED_ASSERT_DELAY_MS  100

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
#ifdef BLINKY_LED_PWM_CONTROL

    blinky_led_pwm_set(BLINKY_LED_0, 0);
    blinky_led_pwm_set(BLINKY_LED_1, 0);
    blinky_led_pwm_set(BLINKY_LED_2, 0);
    blinky_led_pwm_set(BLINKY_LED_3, 0);

    blinky_led_pwm_set(BLINKY_LED_0, 100);
    blinky_led_pwm_set(BLINKY_LED_1, 100);
    while (true)
    {}
#else
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
#endif
}
