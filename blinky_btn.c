#include "blinky_btn.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"

#define BLINKY_BTN_0                        NRF_GPIO_PIN_MAP(1, 6)  /* 0 SW1 */
#define BLINKY_BTN_ACTIVE_STATE             0
#define BLINKY_BTN_DEBOUNCING_TIMEOUT_MS    50
#define BLINKY_BTN_MULTICLICK_TIMEOUT_MS    500

APP_TIMER_DEF(g_timer_debouncing);
APP_TIMER_DEF(g_timer_multiclick);

static volatile bool g_timer_multiclick_in_progress = false;
static volatile uint8_t g_click_cnt = 0;

static click_cb_t g_on_button_click = NULL;
static click_cb_t g_on_button_double_click = NULL;
static click_cb_t g_on_button_triple_click = NULL;

void button_click_handler(void)
{
    g_click_cnt++;
    if (g_timer_multiclick_in_progress)
    {
        app_timer_stop(g_timer_multiclick);
    }
    else
    {
        g_timer_multiclick_in_progress = true;
    }
    
    ASSERT(NRF_SUCCESS == app_timer_start(g_timer_multiclick, APP_TIMER_TICKS(BLINKY_BTN_MULTICLICK_TIMEOUT_MS), NULL));
    
    switch (g_click_cnt)
    {
        case 1:
            if (g_on_button_click)
            {
                g_on_button_click();
            }
            break;
        case 2:
            if (g_on_button_double_click)
            {
                g_on_button_double_click();
            }
            break;
        case 3:
            if (g_on_button_triple_click)
            {
                g_on_button_triple_click();
            }
            break;
        default:
            break;
    }
}

void app_timer_debouncing_handler(void * p_context)
{
    app_timer_stop(g_timer_debouncing);
    if (BLINKY_BTN_ACTIVE_STATE != nrf_gpio_pin_read(BLINKY_BTN_0))
    {
        button_click_handler();
    }
}

void app_timer_multiclick_handler(void * p_context)
{
    app_timer_stop(g_timer_multiclick);
    g_timer_multiclick_in_progress = false;
    g_click_cnt = 0;
}

void button0_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /* start debouncing timer when user released btn */
    if (BLINKY_BTN_ACTIVE_STATE != nrf_gpio_pin_read(BLINKY_BTN_0))
    {
        ASSERT(NRF_SUCCESS == app_timer_start(g_timer_debouncing, APP_TIMER_TICKS(BLINKY_BTN_DEBOUNCING_TIMEOUT_MS), NULL));
    }
}

void blinky_btns_init(click_cb_t on_click, click_cb_t on_double_click, click_cb_t on_triple_click)
{
    g_on_button_click = on_click;
    g_on_button_double_click = on_double_click;
    g_on_button_triple_click = on_triple_click;

    ASSERT(NRFX_SUCCESS == nrfx_gpiote_init());
    nrfx_gpiote_in_config_t btn_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    btn_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BLINKY_BTN_0, &btn_config, button0_event_handler);
    nrfx_gpiote_in_event_enable(BLINKY_BTN_0, true);
    
    ASSERT(NRF_SUCCESS == app_timer_create(&g_timer_debouncing, APP_TIMER_MODE_SINGLE_SHOT, app_timer_debouncing_handler));
    ASSERT(NRF_SUCCESS == app_timer_create(&g_timer_multiclick, APP_TIMER_MODE_SINGLE_SHOT, app_timer_multiclick_handler));
}