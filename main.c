#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "boards.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_assert.h"

/* LEDs */
#define BLINKY_LED_0            NRF_GPIO_PIN_MAP(0, 6)  /* LED1   Green */
#define BLINKY_LED_1            NRF_GPIO_PIN_MAP(0, 8)  /* LED2_R Red */
#define BLINKY_LED_2            NRF_GPIO_PIN_MAP(1, 9)  /* LED2_G Green */
#define BLINKY_LED_3            NRF_GPIO_PIN_MAP(0, 12) /* LED2_B Blue */
#define BLINKY_LED_ACTIVE_STATE 0

/* BUTTON */
#define BLINKY_BTN_0            NRF_GPIO_PIN_MAP(1, 6)  /* 0 SW1 */
#define BLINKY_BTN_ACTIVE_STATE 0

#define BLINKY_LED_DELAY_MS         500
#define BLINKY_CIRCLE_DELAY_MS      (BLINKY_LED_DELAY_MS * 3)
#define BLINKY_DELAY_STEP_MS        (BLINKY_LED_DELAY_MS / 10)  /* sampling rate */
#define BLINKY_LED_ASSERT_DELAY_MS  100

/* stock number of a board (#ABCD) */
#define BLINKY_SN_A         6
#define BLINKY_SN_B         5
#define BLINKY_SN_C         8
#define BLINKY_SN_D         4

typedef enum
{ 
    NODE_LED_STATE,
    NODE_DELAY
} node_type_t;

typedef struct
{
    uint32_t led;
    bool state; /* true = ON, false = OFF */
} led_t;

typedef struct
{
    uint32_t delay;
    uint32_t passed;
} delay_t;

typedef union
{
    delay_t delay_data;
    led_t led_data;
} node_data_t;

typedef struct
{
    node_type_t type;
    node_data_t data;
 } node_t;

typedef struct
{
    uint32_t queue_size;
    node_t queue_states[(BLINKY_SN_A * 4) + (BLINKY_SN_B * 4) + (BLINKY_SN_C * 4) + (BLINKY_SN_D * 4) + 1];
    uint32_t queue_index;
 } queue_t;

void blinky_led_on(uint32_t led)
{
    nrf_gpio_pin_write(led, BLINKY_LED_ACTIVE_STATE);
}

void blinky_led_off(uint32_t led)
{
    nrf_gpio_pin_write(led, !BLINKY_LED_ACTIVE_STATE);
}

void blinky_all_led_off(void)
{
    blinky_led_off(BLINKY_LED_0);
    blinky_led_off(BLINKY_LED_1);
    blinky_led_off(BLINKY_LED_2);
    blinky_led_off(BLINKY_LED_3);
}

void blinky_leds_init(void)
{
    /* Configure GPIO for leds */
    nrf_gpio_cfg_output(BLINKY_LED_0);
    nrf_gpio_cfg_output(BLINKY_LED_1);
    nrf_gpio_cfg_output(BLINKY_LED_2);
    nrf_gpio_cfg_output(BLINKY_LED_3);

    /* Switch Off all leds to make sure the initial state */
    blinky_all_led_off();
}

void blinky_btn_init(void)
{
    nrf_gpio_cfg_input(BLINKY_BTN_0, NRF_GPIO_PIN_PULLUP);
}

void blinky_put_blink(queue_t* queue, uint32_t led, uint32_t blink_repeat, uint32_t first_index)
{
    ASSERT(queue != NULL);

    for (uint32_t i = 0; i < blink_repeat; ++i)
    {
        node_t* node = &(queue->queue_states[first_index++]);
        node->type = NODE_LED_STATE;
        node->data.led_data.state = true;
        node->data.led_data.led = led;

        node = &(queue->queue_states[first_index++]);
        node->type = NODE_DELAY;
        node->data.delay_data.delay = BLINKY_LED_DELAY_MS;
        node->data.delay_data.passed = 0;

        node = &(queue->queue_states[first_index++]);
        node->type = NODE_LED_STATE;
        node->data.led_data.state = false;
        node->data.led_data.led = led;

        node = &(queue->queue_states[first_index++]);
        node->type = NODE_DELAY;
        node->data.delay_data.delay = BLINKY_LED_DELAY_MS;
        node->data.delay_data.passed = 0;
    }
}

void blinky_array_init(queue_t* queue)
{
    ASSERT(queue != NULL);

    queue->queue_size = NRFX_ARRAY_SIZE(queue->queue_states);

    blinky_put_blink(queue, BLINKY_LED_0, BLINKY_SN_A, 0);
    blinky_put_blink(queue, BLINKY_LED_1, BLINKY_SN_B, (BLINKY_SN_A * 4));
    blinky_put_blink(queue, BLINKY_LED_2, BLINKY_SN_C, (BLINKY_SN_A * 4) + (BLINKY_SN_B * 4));
    blinky_put_blink(queue, BLINKY_LED_3, BLINKY_SN_D, (BLINKY_SN_A * 4) + (BLINKY_SN_B * 4) + (BLINKY_SN_C * 4));

    /*large delay*/
    node_t* node = &(queue->queue_states[queue->queue_size - 1]);
    node->type = NODE_DELAY;
    node->data.delay_data.delay = BLINKY_CIRCLE_DELAY_MS;
    node->data.delay_data.passed = 0;
}

void blinky_next_state_if_possible(queue_t* queue)
{
    ASSERT(queue != NULL);

    node_t* current_state = &(queue->queue_states[queue->queue_index]);

    switch(current_state->type)
    {
        case NODE_DELAY:
        {
            if(current_state->data.delay_data.passed >= current_state->data.delay_data.delay)
            {
                current_state->data.delay_data.passed = 0;
                queue->queue_index++;
                queue->queue_index %= queue->queue_size;
            }
            break;
        }
        case NODE_LED_STATE:
        {
            queue->queue_index++;
            queue->queue_index %= queue->queue_size;
            break;
        }
        default:
        break;
    }
}

void blinky_play_sequence(queue_t* queue)
{
    ASSERT(queue != NULL);

    node_t* current_state = &(queue->queue_states[queue->queue_index]);

    switch(current_state->type)
    {
        case NODE_DELAY:
        {
            nrf_delay_ms(BLINKY_DELAY_STEP_MS);
            current_state->data.delay_data.passed += BLINKY_DELAY_STEP_MS;
            break;
        }
        case NODE_LED_STATE:
        {
            current_state->data.led_data.state ? blinky_led_on(current_state->data.led_data.led) : blinky_led_off(current_state->data.led_data.led);
            break;
        }
        default:
        break;
    }
    blinky_next_state_if_possible(queue);
}

void assert_nrf_callback(uint16_t line_num, const uint8_t * file_name)
{
    blinky_all_led_off();

    while(true)
    {
        blinky_led_on(BLINKY_LED_0);
        nrf_delay_ms(BLINKY_LED_ASSERT_DELAY_MS);
        blinky_led_off(BLINKY_LED_0);
        
        blinky_led_on(BLINKY_LED_1);
        nrf_delay_ms(BLINKY_LED_ASSERT_DELAY_MS);
        blinky_led_off(BLINKY_LED_1);
    }
}

/* Application main entry.*/
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_NONE);
    blinky_leds_init();
    blinky_btn_init();

    queue_t queue = { 0 };
    blinky_array_init(&queue);

    while(true)
    {
        if (BLINKY_BTN_ACTIVE_STATE == nrf_gpio_pin_read(BLINKY_BTN_0))
        {
            blinky_play_sequence(&queue);
        }
    }

    return 0;
}
