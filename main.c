#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"
#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "nrf_drv_clock.h"
#include "nrfx_gpiote.h"

#include "blinky_led.h"
#include "blinky_btn.h"
#include "blinky_assert.h"

#define BLINKY_LED_0        0
#define BLINKY_LED_1        1
#define BLINKY_LED_2        2
#define BLINKY_LED_3        3


#define BLINKY_LED_DELAY_MS                 1000
#define BLINKY_CIRCLE_DELAY_MS              (BLINKY_LED_DELAY_MS * 3)

/* stock number of a board (#ABCD) */
#define BLINKY_SN_A         6
#define BLINKY_SN_B         5
#define BLINKY_SN_C         8
#define BLINKY_SN_D         4

typedef struct
{
    uint32_t time_begin;
    uint32_t time_current;
    uint32_t time_left;

} delay_time_t;

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

static volatile bool g_timer_delay_idle = false;
static volatile bool g_play = false;

APP_TIMER_DEF(g_timer_delay);

static delay_time_t g_delay_time = { 0 };

static void blinky_put_blink(queue_t* queue, uint32_t led_idx, uint32_t blink_repeat, uint32_t first_index)
{
    ASSERT(NULL != queue);

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
    ASSERT(NULL != queue);

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

void blinky_pause_delay_timer(void)
{
    app_timer_stop(g_timer_delay);
    uint32_t time_end = app_timer_cnt_get();
    
    ASSERT(time_end > g_delay_time.time_begin);
    uint32_t time_diff = app_timer_cnt_diff_compute(time_end, g_delay_time.time_begin);
    
    ASSERT( APP_TIMER_TICKS(g_delay_time.time_current) > time_diff);
    g_delay_time.time_left = APP_TIMER_TICKS(g_delay_time.time_current) - time_diff;
    
    NRF_LOG_INFO("pause time, left: %u ticks", g_delay_time.time_left);
}

void blinky_resume_delay_timer(void)
{
    NRF_LOG_INFO("resume time, left: %u ticks", g_delay_time.time_left);
    ASSERT(NRF_SUCCESS == app_timer_start(g_timer_delay, g_delay_time.time_left, NULL));
    g_delay_time.time_begin = app_timer_cnt_get();
}

void blinky_on_button_click(void)
{
    NRF_LOG_INFO("blinky_on_button_click");
}

void blinky_on_button_double_click(void)
{
    NRF_LOG_INFO("blinky_on_button_double_click");
    g_play = !g_play;

    if (g_timer_delay_idle)
    {
        if (g_play)
        {
            /* start playing during dalay event, "resume" delay timer */
            blinky_resume_delay_timer();
        }
        else
        {
            /* stop playing during dalay event, "pause" delay timer */
            blinky_pause_delay_timer();
        }
    }
}

void blinky_on_button_triple_click(void)
{
    NRF_LOG_INFO("blinky_on_button_triple_click");
}

void app_timer_delay_handler(void * p_context)
{
    app_timer_stop(g_timer_delay);
    g_timer_delay_idle = false;
}

void blinky_go_to_next_node(queue_t* queue)
{
    NRF_LOG_INFO("blinky_go_to_next_node");
    ASSERT(NULL != queue);
    queue->queue_index++;
    queue->queue_index %= queue->queue_size;

    node_t* next_node = &(queue->queue_states[queue->queue_index]);
    switch (next_node->type)        
    {
        case NODE_LED_STATE:
            next_node->data.led_data.need_play = true;
            break;

        case NODE_DELAY:
            g_timer_delay_idle = true;
            ASSERT(NRF_SUCCESS == app_timer_start(g_timer_delay, APP_TIMER_TICKS(next_node->data.delay), queue));
            
            /*save global data to pause delay, cannot pass user data to button0_event_handler */
            g_delay_time.time_current = next_node->data.delay;
            g_delay_time.time_begin = app_timer_cnt_get();
            break;

        default:
        break;
    }
}

void blinky_play_sequence(queue_t* queue, bool play)
{
    ASSERT(NULL != queue);

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
            blinky_go_to_next_node(queue);
        }
    }

    if (current_node->type == NODE_DELAY)
    {
        if (!g_timer_delay_idle)
        {
            blinky_go_to_next_node(queue);
        }
    }
}
void blinky_init(queue_t* queue)
{
    ASSERT(NULL != queue);

    /* Some trick to force app_timer work */
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);

    /* Logs init */
    ASSERT(NRF_SUCCESS == NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    /* Leds init */
    NRF_LOG_INFO("Leds init");
    blinky_soft_leds_init();

    /* Timers init */
    NRF_LOG_INFO("Timers init");
    ASSERT(NRF_SUCCESS == app_timer_init());
    ASSERT(NRF_SUCCESS == app_timer_create(&g_timer_delay, APP_TIMER_MODE_SINGLE_SHOT, app_timer_delay_handler));

    /* Buttons init */
    NRF_LOG_INFO("Buttons init");
    blinky_btns_init(blinky_on_button_click, blinky_on_button_double_click, blinky_on_button_triple_click);

    /* App init */
    NRF_LOG_INFO("App init");
    blinky_array_init(queue);
}

/* Application main entry.*/
int main(void)
{   
    queue_t queue = { 0 };

    blinky_init(&queue);

    /* Main loop */
    NRF_LOG_INFO("Main loop go");
    while (true)
    {
        blinky_soft_leds_loop(g_play);
        blinky_play_sequence(&queue, g_play);

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }

    return 0;
}
