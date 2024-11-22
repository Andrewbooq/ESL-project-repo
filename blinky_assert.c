#include "nrf_delay.h"
#include "blinky_led.h"
#include "blinky_log.h"

#ifdef BLINKY_LED_PWM_CONTROL
#include "blinky_led_pwm.h"
#define BLINKY_LED_ASSERT_ON_PER  100 /* %, duty cycle */
#define BLINKY_LED_ASSERT_OFF_PER 0 /* %, duty cycle */
#endif

#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ " : " S2(__LINE__)

#define BLINKY_LED_ASSERT_DELAY_MS  100

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
#ifdef BLINKY_LED_PWM_CONTROL
    NRF_LOG_INFO(LOCATION);

    blinky_led_pwm_set(BLINKY_LED_0, BLINKY_LED_ASSERT_OFF_PER);
    blinky_led_pwm_set(BLINKY_LED_1, BLINKY_LED_ASSERT_OFF_PER);
    blinky_led_pwm_set(BLINKY_LED_2, BLINKY_LED_ASSERT_OFF_PER);
    blinky_led_pwm_set(BLINKY_LED_3, BLINKY_LED_ASSERT_OFF_PER);

    blinky_led_pwm_set(BLINKY_LED_0, BLINKY_LED_ASSERT_ON_PER);
    blinky_led_pwm_set(BLINKY_LED_1, BLINKY_LED_ASSERT_ON_PER);
    while (true)
    {
    }
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
