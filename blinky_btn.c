#include "nrfx_gpiote.h"
#include "app_timer.h"

#include "blinky_log.h"
#include "blinky_btn.h"

#define BLINKY_BTN_0                        NRF_GPIO_PIN_MAP(1, 6)  /* 0 SW1 */
#define BLINKY_BTN_ACTIVE_STATE             0
#define BLINKY_BTN_DEBOUNCING_TIMEOUT_MS    50
#define BLINKY_BTN_MULTICLICK_TIMEOUT_MS    300
#define BLINKY_BTN_HOLD_TIMEOUT_MS          BLINKY_BTN_MULTICLICK_TIMEOUT_MS

APP_TIMER_DEF(g_timer_debouncing);
APP_TIMER_DEF(g_timer_multiclick);
APP_TIMER_DEF(g_timer_hold);

static volatile bool g_timer_multiclick_in_progress = false;
static volatile uint32_t g_click_cnt = 0;

static click_cb_t g_on_button_hold = NULL;
static click_cb_t g_on_button_release = NULL;
static click_cb_t g_on_button_multi_click = NULL;

void button_click_handler(void)
{
    NRF_LOG_INFO("BTN: button_click_handler");
    g_click_cnt++;
    if (g_timer_multiclick_in_progress)
    {
        app_timer_stop(g_timer_multiclick);
    }
    else
    {
        g_timer_multiclick_in_progress = true;
    }
    ret_code_t res = app_timer_start(g_timer_multiclick, APP_TIMER_TICKS(BLINKY_BTN_MULTICLICK_TIMEOUT_MS), NULL);
    ASSERT(NRF_SUCCESS == res);
}

void button_press_handler(void)
{
    /* Without timer on_button_press is called even user makes multi-click.
    It leaded to performing on_button_press and doing some logic of holding the button (short time)
    when user clickes double click.
    To separate on_button_press/on_button_hold/milti-click make it over timer. */
    ret_code_t res = app_timer_start(g_timer_hold, APP_TIMER_TICKS(BLINKY_BTN_HOLD_TIMEOUT_MS), NULL);
    ASSERT(NRF_SUCCESS == res);
}

void button_release_handler(void)
{
    app_timer_stop(g_timer_hold);
    if (g_on_button_release)
    {
        g_on_button_release(NULL);
    }

    /* Make Click event as soon as user released the button as it implemented in most GUI */
    button_click_handler();
}

void app_timer_debouncing_handler(void * p_context)
{
    if (BLINKY_BTN_ACTIVE_STATE == nrf_gpio_pin_read(BLINKY_BTN_0))
    {
        button_press_handler();
    }
    else
    {
        button_release_handler(); 
    }
}

void app_timer_multiclick_handler(void * p_context)
{
    NRF_LOG_INFO("BTN: app_timer_multiclick_handler");
    UNUSED_VARIABLE(p_context);

    if (g_on_button_multi_click)
    {
        g_on_button_multi_click((void*)g_click_cnt);
    }
    g_timer_multiclick_in_progress = false;
    g_click_cnt = 0;
}

void app_timer_hold_handler(void * p_context)
{
    NRF_LOG_INFO("BTN: app_timer_hold_handler");
    if (g_on_button_hold)
    {
        g_on_button_hold(NULL);
    }
}

void button0_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /* First start debouncing timer*/
    ret_code_t res = app_timer_start(g_timer_debouncing, APP_TIMER_TICKS(BLINKY_BTN_DEBOUNCING_TIMEOUT_MS), NULL);
    ASSERT(NRF_SUCCESS == res);
}

void blinky_btns_init(click_cb_t on_hold, click_cb_t on_release, click_cb_t on_multi_click)
{
    g_on_button_hold = on_hold;
    g_on_button_release = on_release;
    g_on_button_multi_click = on_multi_click;

    nrfx_err_t resx = nrfx_gpiote_init();
    ASSERT(NRFX_SUCCESS == resx);
    
    nrfx_gpiote_in_config_t btn_config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    btn_config.pull = NRF_GPIO_PIN_PULLUP;
    nrfx_gpiote_in_init(BLINKY_BTN_0, &btn_config, button0_event_handler);
    nrfx_gpiote_in_event_enable(BLINKY_BTN_0, true);
    
    ret_code_t res = app_timer_create(&g_timer_debouncing, APP_TIMER_MODE_SINGLE_SHOT, app_timer_debouncing_handler);
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_multiclick, APP_TIMER_MODE_SINGLE_SHOT, app_timer_multiclick_handler);
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_hold, APP_TIMER_MODE_SINGLE_SHOT, app_timer_hold_handler);
    ASSERT(NRF_SUCCESS == res);
}