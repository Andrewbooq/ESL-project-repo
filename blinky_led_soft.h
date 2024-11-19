#ifndef BLINKY_LED_SOFT_H__
#define BLINKY_LED_SOFT_H__

/* need to call app_timer_init first */
void blinky_led_soft_init(void);

/*  led_idx : from 0 to BLINKY_LEDS_ON_BOARD
    blink_time_ms : time to a blink cycle up and down: 0% -> 100% -> 0% */
void blinky_led_soft_on(uint8_t led_idx, uint32_t blink_time_ms);
void blinky_led_soft_off(uint8_t led_idx);

#endif /* BLINKY_LED_SOFT_H__ */