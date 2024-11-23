#ifndef BLINKY_LED_H__
#define BLINKY_LED_H__

#include "nrf_gpio.h"

/* keep here for assert */
#define BLINKY_LED_0_PIN            NRF_GPIO_PIN_MAP(0, 6)  /* LED1   Green */
#define BLINKY_LED_1_PIN            NRF_GPIO_PIN_MAP(0, 8)  /* LED2_R Red */
#define BLINKY_LED_2_PIN            NRF_GPIO_PIN_MAP(1, 9)  /* LED2_G Green */
#define BLINKY_LED_3_PIN            NRF_GPIO_PIN_MAP(0, 12) /* LED2_B Blue */

#define BLINKY_LED_0        0
#define BLINKY_LED_1        1
#define BLINKY_LED_2        2
#define BLINKY_LED_3        3

#define BLINKY_LED_R        BLINKY_LED_1
#define BLINKY_LED_G        BLINKY_LED_2
#define BLINKY_LED_B        BLINKY_LED_3

#define BLINKY_LEDS_ON_BOARD        4

#define BLINKY_LED_ACTIVE_STATE     0

/*  led_idx : from 0 to BLINKY_LEDS_ON_BOARD */
bool blinky_check_idx(uint8_t idx);
void blinky_leds_init(void);
void blinky_led_on(uint8_t led_idx);
void blinky_led_off(uint8_t led_idx);
void blinky_led_invert(uint8_t led_idx);
void blinky_all_led_off(void);

#endif /* BLINKY_LED_H__ */