#include "app_timer.h"
#include "blinky_log.h"

#include "blinky_led.h"
#include "blinky_led_soft.h"
#include "blinky_led_pwm.h"

APP_TIMER_DEF(g_timer_led0);
APP_TIMER_DEF(g_timer_led1);
APP_TIMER_DEF(g_timer_led2);
APP_TIMER_DEF(g_timer_led3);

#define BLINKY_DARK_STEPS 50

typedef struct
{
    volatile uint8_t duty_cycle;
    volatile int8_t dark_step;
    volatile bool up;
} led_t;

static led_t g_blink_leds[BLINKY_LEDS_ON_BOARD] = { 0 };


void app_timer_led_handler(void * p_context)
{
    uint32_t led_idx = (uint32_t)p_context;
    ASSERT(blinky_check_idx(led_idx));

    if (g_blink_leds[led_idx].up)
    {
        g_blink_leds[led_idx].duty_cycle++;
        if (g_blink_leds[led_idx].duty_cycle == 100)
        {
            g_blink_leds[led_idx].up = false;
        }
    }
    else
    {
        if (g_blink_leds[led_idx].duty_cycle == 0)
        {
            if (g_blink_leds[led_idx].dark_step > 0)
            {
                g_blink_leds[led_idx].dark_step--;
            }
            else
            {
                /* small tweak to make blinking more beutiful */
                g_blink_leds[led_idx].dark_step = BLINKY_DARK_STEPS;
                g_blink_leds[led_idx].up = true;
            }
        }
        else
        {
            g_blink_leds[led_idx].duty_cycle--;
        }
    }

    blinky_led_pwm_set(led_idx, g_blink_leds[led_idx].duty_cycle);
}

void blinky_led_soft_init(void)
{
    blinky_led_pwm_init();

    ret_code_t res = app_timer_create(&g_timer_led0, APP_TIMER_MODE_REPEATED, app_timer_led_handler);
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_led1, APP_TIMER_MODE_REPEATED, app_timer_led_handler);
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_led2, APP_TIMER_MODE_REPEATED, app_timer_led_handler);
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_led3, APP_TIMER_MODE_REPEATED, app_timer_led_handler);
    ASSERT(NRF_SUCCESS == res);
}

void blinky_led_soft_on(uint8_t led_idx , uint32_t blink_time_ms)
{
    ASSERT(blinky_check_idx(led_idx));
    
    /* Time step of 1% of a duty cycle up and down */
    uint32_t step = blink_time_ms / (2 * 100);
    ret_code_t res = NRF_SUCCESS;

    /* First stop timer to reconfig blinking parameters */  
    switch(led_idx)
    {
        case BLINKY_LED_0: app_timer_stop(g_timer_led0); break;
        case BLINKY_LED_1: app_timer_stop(g_timer_led1); break;
        case BLINKY_LED_2: app_timer_stop(g_timer_led2); break;
        case BLINKY_LED_3: app_timer_stop(g_timer_led3); break;
        default: ASSERT(false);                          break;
    }

    /* Initial state */
    g_blink_leds[led_idx].up = true;
    g_blink_leds[led_idx].duty_cycle = 0;
    g_blink_leds[led_idx].dark_step = BLINKY_DARK_STEPS;
    blinky_led_pwm_set(led_idx, 0);

    switch(led_idx)
    {
        case BLINKY_LED_0:
        {
            res = app_timer_start(g_timer_led0, APP_TIMER_TICKS(step), (void*)BLINKY_LED_0);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_1:
        {
            res = app_timer_start(g_timer_led1, APP_TIMER_TICKS(step), (void*)BLINKY_LED_1);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_2:
        {
            res = app_timer_start(g_timer_led2, APP_TIMER_TICKS(step), (void*)BLINKY_LED_2);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_3:
        {
            res = app_timer_start(g_timer_led3, APP_TIMER_TICKS(step), (void*)BLINKY_LED_3);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        default:
            ASSERT(false);
            break;
    }
}

void blinky_led_soft_off(uint8_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    
    ret_code_t res = NRF_SUCCESS;

    g_blink_leds[led_idx].duty_cycle = 0;
    blinky_led_pwm_set(led_idx, 0);

    switch (led_idx)
    {
        case BLINKY_LED_0:
        {
            res = app_timer_stop(g_timer_led0);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_1:
        {
            res = app_timer_stop(g_timer_led1);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_2:
        {
            res = app_timer_stop(g_timer_led2);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        case BLINKY_LED_3:
        {
            res = app_timer_stop(g_timer_led3);
            ASSERT(NRF_SUCCESS == res);
            break;
        }
        default:
            break;
    }
}