#include "blinky_led.h"

static uint32_t g_leds[] = { BLINKY_LED_0_PIN, BLINKY_LED_1_PIN, BLINKY_LED_2_PIN, BLINKY_LED_3_PIN };


bool blinky_check_idx(uint8_t idx)
{
    return idx < BLINKY_LEDS_ON_BOARD;
}

void blinky_leds_init(void)
{
    /* Configure GPIO for leds */
    nrf_gpio_cfg_output(BLINKY_LED_0_PIN);
    nrf_gpio_cfg_output(BLINKY_LED_1_PIN);
    nrf_gpio_cfg_output(BLINKY_LED_2_PIN);
    nrf_gpio_cfg_output(BLINKY_LED_3_PIN);

    /* Switch Off all leds to make sure the initial state */
    blinky_all_led_off();
}

void blinky_led_on(uint8_t led_idx)
{   
    ASSERT(blinky_check_idx(led_idx));
    uint32_t led = g_leds[led_idx];
    nrf_gpio_pin_write(led, BLINKY_LED_ACTIVE_STATE);
}

void blinky_led_off(uint8_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    uint32_t led = g_leds[led_idx];
    nrf_gpio_pin_write(led, !BLINKY_LED_ACTIVE_STATE);
}

void blinky_led_invert(uint8_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    uint32_t led = g_leds[led_idx];
    uint32_t state = nrf_gpio_pin_out_read(led);
    if (state == BLINKY_LED_ACTIVE_STATE)
    {
        nrf_gpio_pin_write(led, !BLINKY_LED_ACTIVE_STATE);
    }
    else
    {
        nrf_gpio_pin_write(led, BLINKY_LED_ACTIVE_STATE);
    }
}

void blinky_all_led_off(void)
{
    for (uint32_t i = 0; i < BLINKY_LEDS_ON_BOARD; ++i)
    {
        blinky_led_off(i);
    }
}
