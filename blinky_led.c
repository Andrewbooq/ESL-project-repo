#include "blinky_led.h"
#include "nrfx_systick.h"

#define BLINKY_LEDS_ON_BOARD        4

#define BLINKY_PWM_FREQ_HZ          1000                            /* Hz frequency of PWM */
#define BLINKY_PWM_PERIOD_US        (1000000 / BLINKY_PWM_FREQ_HZ)  /* us period of PWM*/

#define BLINKY_SOFT_PERIOD_MS       1000                                    /* ms time to get brightness from min to max (or max to min)  */
#define BLINKY_SOFT_STEP_US         (BLINKY_SOFT_PERIOD_MS * 1000 / 100)    /* us time for 1% of brightness */

typedef enum {
    LED_POLL_ON,
    LED_POLL_OFF,
    LED_ON,
    LED_OFF,
} led_state_t;

typedef struct
{
    bool on;
    bool up;
    uint32_t time_on;
    uint32_t time_off;
    uint8_t duty_cycle;
    uint8_t repeat;
    led_state_t state;
    nrfx_systick_state_t pwm_time;
    nrfx_systick_state_t soft_time;
} soft_led_t;

static soft_led_t g_soft_leds[BLINKY_LEDS_ON_BOARD] = { 0 };
static uint32_t g_leds[] = { BLINKY_LED_0_PIN, BLINKY_LED_1_PIN, BLINKY_LED_2_PIN, BLINKY_LED_3_PIN };
static bool g_pause_active = true;

bool blinky_check_idx(uint32_t idx)
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

void blinky_led_on(uint32_t led_idx)
{   
    ASSERT(blinky_check_idx(led_idx));
    uint32_t led = g_leds[led_idx];
    nrf_gpio_pin_write(led, BLINKY_LED_ACTIVE_STATE);
}

void blinky_led_off(uint32_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    uint32_t led = g_leds[led_idx];
    nrf_gpio_pin_write(led, !BLINKY_LED_ACTIVE_STATE);
}

void blinky_led_invert(uint32_t led_idx)
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

void blinky_soft_leds_init(void)
{
    blinky_leds_init();
    nrfx_systick_init();
}

void blinky_soft_led_on(uint32_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    
    // initial states
    g_soft_leds[led_idx].on = true;
    g_soft_leds[led_idx].up = true;
    g_soft_leds[led_idx].time_on = 0; /* us */
    g_soft_leds[led_idx].time_off = BLINKY_PWM_PERIOD_US;
    g_soft_leds[led_idx].duty_cycle = 0; /* % */
    g_soft_leds[led_idx].repeat = 1; /* cnt */
    g_soft_leds[led_idx].state = LED_ON;
    nrfx_systick_get(&(g_soft_leds[led_idx].pwm_time));
    g_soft_leds[led_idx].soft_time = g_soft_leds[led_idx].pwm_time;
}

void blinky_soft_led_off(uint32_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    blinky_led_off(led_idx);
    g_soft_leds[led_idx].on = false;
}

bool blinky_soft_led_done(uint32_t led_idx)
{
    ASSERT(blinky_check_idx(led_idx));
    return (g_soft_leds[led_idx].repeat == 0);
}

void blinky_soft_leds_loop(bool play)
{
    for (uint32_t i = 0; i < BLINKY_LEDS_ON_BOARD; ++i)
    {
        soft_led_t* soft_led = &(g_soft_leds[i]);
        if ( !soft_led->on || (soft_led->repeat == 0))
        {
            continue;
        }

        /* pause */
        if (!play)
        {
            if (!g_pause_active) /* keep light or turn off */
            {
                soft_led->state = LED_OFF;
            }
        }

        // each BLINKY_SOFT_STEP_US (1%) change brightness
        if (nrfx_systick_test(&(soft_led->soft_time), BLINKY_SOFT_STEP_US))
        {
            if (soft_led->up)
            {
                if (play)
                    soft_led->duty_cycle++;
            }
            else
            {
                if (play)
                    soft_led->duty_cycle--;

                if (soft_led->duty_cycle == 0)
                {
                    soft_led->repeat--;
                }
            }
            
            /* recalculate PWM times */
            soft_led->time_on = soft_led->duty_cycle * BLINKY_PWM_PERIOD_US / 100; // us
            soft_led->time_off = BLINKY_PWM_PERIOD_US - soft_led->time_on; // us
            nrfx_systick_get(&(soft_led->soft_time));

            // reverse changing brightness after applying the last step (0 or 100 %)
            if ( 100 == soft_led->duty_cycle )
            {
                soft_led->up = false;
            }

            if ( 0 == soft_led->duty_cycle )
            {
                soft_led->up = true;
            }
        }

        // PWM process
        switch(soft_led->state)
        {
            case LED_POLL_ON:
                if(nrfx_systick_test(&(soft_led->pwm_time), soft_led->time_on))
                {
                    soft_led->state = LED_OFF;
                    nrfx_systick_get(&(soft_led->pwm_time));
                }
                break;
            case LED_POLL_OFF:
                if(nrfx_systick_test(&(soft_led->pwm_time), soft_led->time_off))
                {
                    soft_led->state = LED_ON;
                    nrfx_systick_get(&(soft_led->pwm_time));
                }
                break;
            case LED_ON:
                soft_led->state = LED_POLL_ON;
                blinky_led_on(i);
                break;
            case LED_OFF:
                soft_led->state = LED_POLL_OFF;
                blinky_led_off(i);                
                break;
            default:
                break;
        }
    }
}