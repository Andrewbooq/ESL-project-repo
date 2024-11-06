#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "app_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_delay.h"
#include "boards.h"
#include "blinky_led.h"
#include "blinky_assert.h"

#define BLINKY_LED_0    0
#define BLINKY_LED_1    1
#define BLINKY_LED_2    2
#define BLINKY_LED_3    3

/* BUTTON */
#define BLINKY_BTN_0            NRF_GPIO_PIN_MAP(1, 6)  /* 0 SW1 */
#define BLINKY_BTN_ACTIVE_STATE 0

#define BLINKY_LED_DELAY_MS         300
#define BLINKY_CIRCLE_DELAY_MS      (BLINKY_LED_DELAY_MS * 3)
#define BLINKY_DELAY_STEP_MS        (BLINKY_LED_DELAY_MS / 10)  /* sampling rate */

/* stock number of a board (#ABCD) */
#define BLINKY_SN_A         6
#define BLINKY_SN_B         5
#define BLINKY_SN_C         8
#define BLINKY_SN_D         4

static volatile bool g_timer_idle;
APP_TIMER_DEF(g_timer_one);

typedef enum
{ 
    NODE_LED_STATE,
    NODE_DELAY
} node_type_t;

typedef struct
{
    uint32_t led_idx; /* from 0 to BLINKY_LEDS_ON_BOARD */
    bool need_play;
} led_t;

typedef union
{
    uint32_t delay;
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
    node_t queue_states[ (BLINKY_SN_A * 2) + (BLINKY_SN_B * 2) + (BLINKY_SN_C * 2) + (BLINKY_SN_D * 2) + 1 ]; /* 2 = blink + delay after blink, 1 large delay at the end of queue*/
    uint32_t queue_index;
 } queue_t;

void blinky_btn_init(void)
{
    nrf_gpio_cfg_input(BLINKY_BTN_0, NRF_GPIO_PIN_PULLUP);
}

void blinky_put_blink(queue_t* queue, uint32_t led_idx, uint32_t blink_repeat, uint32_t first_index)
{
    ASSERT(queue != NULL);

    for (uint32_t i = 0; i < blink_repeat; ++i)
    {
        node_t* node = &(queue->queue_states[first_index++]);
        node->type = NODE_LED_STATE;
        node->data.led_data.need_play = true;
        node->data.led_data.led_idx = led_idx;
        
        node = &(queue->queue_states[first_index++]);
        node->type = NODE_DELAY;
        node->data.delay = BLINKY_LED_DELAY_MS;
    }
}

void blinky_array_init(queue_t* queue)
{
    ASSERT(queue != NULL);

    queue->queue_size = NRFX_ARRAY_SIZE(queue->queue_states);

    blinky_put_blink(queue, BLINKY_LED_0, BLINKY_SN_A, 0);
    blinky_put_blink(queue, BLINKY_LED_1, BLINKY_SN_B, (BLINKY_SN_A * 2));
    blinky_put_blink(queue, BLINKY_LED_2, BLINKY_SN_C, (BLINKY_SN_A * 2) + (BLINKY_SN_B * 2));
    blinky_put_blink(queue, BLINKY_LED_3, BLINKY_SN_D, (BLINKY_SN_A * 2) + (BLINKY_SN_B * 2) + (BLINKY_SN_C * 2));

    /*large delay*/
    node_t* node = &(queue->queue_states[queue->queue_size - 1]);
    node->type = NODE_DELAY;
    node->data.delay = BLINKY_CIRCLE_DELAY_MS;
}

void app_timer_timeout_handler(void * p_context)
{
    app_timer_stop(g_timer_one);
    g_timer_idle = false;
}

void blinky_prepare_next_node(queue_t* queue)
{
    node_t* next_node = &(queue->queue_states[queue->queue_index]);
    switch(next_node->type)        
    {
        case NODE_LED_STATE:
            next_node->data.led_data.need_play = true;
            break;

        case NODE_DELAY:
            g_timer_idle = true;
            ASSERT(NRF_SUCCESS == app_timer_start(g_timer_one, APP_TIMER_TICKS(next_node->data.delay), NULL));
            break;

        default:
        break;
    }
}

void blinky_play_sequence(queue_t* queue, bool play)
{
    ASSERT(queue != NULL);

    node_t* current_node = &(queue->queue_states[queue->queue_index]);

    if (play && current_node->type == NODE_LED_STATE)
    {
        // start blink playing
        if (current_node->data.led_data.need_play)
        {
            blinky_soft_led_on(current_node->data.led_data.led_idx);
            current_node->data.led_data.need_play = false;
        }

        // blink playing finished
        if (blinky_soft_led_done(current_node->data.led_data.led_idx))
        {
            blinky_soft_led_off(current_node->data.led_data.led_idx);

            queue->queue_index++;
            queue->queue_index %= queue->queue_size;

            blinky_prepare_next_node(queue);
        }
    }

    if (current_node->type == NODE_DELAY)
    {
        if (!g_timer_idle)
        {
            queue->queue_index++;
            queue->queue_index %= queue->queue_size;;

            blinky_prepare_next_node(queue);
        }
    }
}

/* Application main entry.*/
int main(void)
{
    /* Configure board. */
    bsp_board_init(BSP_INIT_NONE);

    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
    
    blinky_soft_leds_init();
    
    blinky_btn_init();

    ASSERT(NRF_SUCCESS == app_timer_init());
    ASSERT(NRF_SUCCESS == app_timer_create(&g_timer_one, APP_TIMER_MODE_SINGLE_SHOT, app_timer_timeout_handler));

    g_timer_idle = false;

    queue_t queue = { 0 };
    blinky_array_init(&queue);
    
    bool play = false;
    
    while (true)
    {
        blinky_soft_leds_loop(play);
        blinky_play_sequence(&queue, play);

        if (BLINKY_BTN_ACTIVE_STATE == nrf_gpio_pin_read(BLINKY_BTN_0))
        {
            if(g_timer_idle)
                app_timer_resume();

            play = true;
        }
        else
        {
            if(g_timer_idle)
                app_timer_pause();

            play = false;
        }
    }

    return 0;
}
