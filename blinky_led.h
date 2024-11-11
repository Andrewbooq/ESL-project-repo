#ifndef BLINKY_LED_H__
#define BLINKY_LED_H__

#include "nrf_gpio.h"
#include "nrfx_systick.h"

/* keep here fo assert */
#define BLINKY_LED_0_PIN            NRF_GPIO_PIN_MAP(0, 6)  /* LED1   Green */
#define BLINKY_LED_1_PIN            NRF_GPIO_PIN_MAP(0, 8)  /* LED2_R Red */
#define BLINKY_LED_2_PIN            NRF_GPIO_PIN_MAP(1, 9)  /* LED2_G Green */
#define BLINKY_LED_3_PIN            NRF_GPIO_PIN_MAP(0, 12) /* LED2_B Blue */
#define BLINKY_LED_ACTIVE_STATE     0

void blinky_leds_init(void);
void blinky_led_on(uint32_t led_idx);
void blinky_led_off(uint32_t led_idx);
void blinky_led_invert(uint32_t led_idx);
void blinky_all_led_off(void);

void blinky_soft_leds_init(void);
void blinky_soft_led_on(uint32_t led_idx);
void blinky_soft_led_off(uint32_t led_idx);
bool blinky_soft_led_done(uint32_t led_idx);
void blinky_soft_leds_loop(bool play);

#endif /* BLINKY_LED_H__ */