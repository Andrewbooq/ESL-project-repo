#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include <math.h>

#define BLINKY_LED_DELAY 300
#define BLINKY_CIRCLE_DELAY (BLINKY_LED_DELAY * 3)
#define BLINKY_BOARD_NUMBER 6584
#define BLINKY_GET_DIGIT(number, pos) ((number / (int)pow(10, pos)) % 10) /* Get out digit from a number right to left, starting from 0 */

/* Function to blink the certain len the certain times. */
void blinky_led_blink(uint32_t led_idx, size_t blink_repeat)
{
    for (size_t i = 0; i < blink_repeat; ++i)
    {
        bsp_board_led_on(led_idx);
        nrf_delay_ms(BLINKY_LED_DELAY);
        bsp_board_led_off(led_idx);
        nrf_delay_ms(BLINKY_LED_DELAY);
    }
}

/* Function for application main entry.*/
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_LEDS);
    
    /* Get out digits from the board number*/
    size_t p0 = BLINKY_GET_DIGIT(BLINKY_BOARD_NUMBER, 3);
	size_t p1 = BLINKY_GET_DIGIT(BLINKY_BOARD_NUMBER, 2);
    size_t p2 = BLINKY_GET_DIGIT(BLINKY_BOARD_NUMBER, 1);
    size_t p3 = BLINKY_GET_DIGIT(BLINKY_BOARD_NUMBER, 0);

    while (true)
    {
        blinky_led_blink(BSP_BOARD_LED_0, p0);
        blinky_led_blink(BSP_BOARD_LED_1, p1);
        blinky_led_blink(BSP_BOARD_LED_2, p2);
        blinky_led_blink(BSP_BOARD_LED_3, p3);
        nrf_delay_ms(BLINKY_CIRCLE_DELAY);
    }

    return 0;
}

/**
 *@}
 **/
