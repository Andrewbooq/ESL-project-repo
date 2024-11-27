#ifndef BLINKY_LED_PWM_H__
#define BLINKY_LED_PWM_H__

/*  After setup using led pins over PWM driver the direct control of led pins (nrf_gpio_pin_write) STOP WORKING.
    As result these functions don't work any more:
    blinky_led_on,
    blinky_led_off,
    blinky_led_invert,
    blinky_all_led_off.
    To control leds just use blinky_led_pwm_set with appropriate value (duty_cycle).
    
    ASSERT: assert_nrf_callback led control over nrf_gpio_pin_write stop working.
    Enable define BLINKY_LED_PWM_CONTROL to switch led handling in asserts over PWM driver.
*/
 
void blinky_led_pwm_init(void);

/*  led_idx : from 0 to BLINKY_LEDS_ON_BOARD
    percent % : from 0 to 100 */
void blinky_led_pwm_set(uint8_t led_idx, uint8_t percent);

#endif /* BLINKY_LED_PWM_H__ */