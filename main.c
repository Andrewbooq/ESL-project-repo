#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "app_timer.h"

#include "blinky_log.h"

#if defined(ESTC_USB_CLI_ENABLED) 
#if (ESTC_USB_CLI_ENABLED == 1)
#define USB_CLI_ENABLED
#endif
#endif

#include "blinky_types.h"
#include "blinky_led.h"
#include "blinky_led_pwm.h"
#include "blinky_led_soft.h"
#include "blinky_color.h"
#include "blinky_nvmc.h"
#include "blinky_btn.h"
#include "blinky_cdc_acm.h"
#include "blinky_command.h"

/* stock number of a board (DEVICE_ID=#ABCD) */
#define BLINKY_SN_A         6
#define BLINKY_SN_B         5
#define BLINKY_SN_C         8
#define BLINKY_SN_D         4

#define BLINKY_STATE_FAST_BLINK_MS  800
#define BLINKY_STATE_SLOW_BLINK_MS  3000

#define BLINKY_VELOCITY_MS          50 /* timeout between steps of moving through colors */

APP_TIMER_DEF(g_timer_move);

static data_t g_data =
{
    .state = T_VIEW,
    .move_s_up = false,
    .move_v_up = false,
    .need_save = false,
    .hsv = 
    {
        /* DEVICE_ID=6584
           Last digits: 84
           Hue: 84% => 360 * 0.84 = 302° */
        .h = ((BLINKY_SN_C * 10.f) + BLINKY_SN_D)  * 360.f / 100.f,
        .s = 100.f,
        .v = 100.f
    },
    .saved_hsv = 
    {
        .h = 50.f,
        .s = 50.f,
        .v = 50.f 
    }
};

STATIC_ASSERT(sizeof(g_data.hsv) % sizeof(uint32_t) == 0, "struct must be aligned to 32 bit word");

static char* blinky_state_to_str(state_t state)
{
    switch(state)
    {
        case T_VIEW:            return "VIEW";
        case T_EDIT_HUE:        return "EDIT HUE";
        case T_EDIT_SATURATION: return "EDIT SATURATION";
        case T_EDIT_BRIGHTNESS: return "EDIT BRIGHTNESS";
        case T_COUNT:           // fall-through
        default:                return "UNKNOWN STATE";
    }
}

static void blinky_state_to_led(state_t state)
{
    switch(state)
    {
        case T_VIEW:
            blinky_led_soft_off(BLINKY_LED_0);
            break;
        case T_EDIT_HUE:
            blinky_led_soft_on(BLINKY_LED_0, BLINKY_STATE_SLOW_BLINK_MS);
            break;
        case T_EDIT_SATURATION:
            blinky_led_soft_on(BLINKY_LED_0, BLINKY_STATE_FAST_BLINK_MS);
            break;
        case T_EDIT_BRIGHTNESS:
            blinky_led_soft_off(BLINKY_LED_0);
            blinky_led_pwm_set(BLINKY_LED_0, 100);
            break;
        case T_COUNT: // fall-through
        default:
            ASSERT(false);
            break;
    }
}
void blinky_on_button_hold(void * p_context)
{
    NRF_LOG_INFO("MAIN: blinky_on_button_hold");
    UNUSED_VARIABLE(p_context);

    NRF_LOG_INFO("MAIN: blinky_on_button_hold: BLINKY_VELOCITY_MS = %u", BLINKY_VELOCITY_MS);

    ret_code_t res = app_timer_start(g_timer_move, APP_TIMER_TICKS(BLINKY_VELOCITY_MS), NULL);
    ASSERT(NRF_SUCCESS == res);
}

void blinky_on_button_release(void * p_context)
{
    NRF_LOG_INFO("MAIN: blinky_on_button_release");
    UNUSED_VARIABLE(p_context);

    ret_code_t res = app_timer_stop(g_timer_move);
    ASSERT(NRF_SUCCESS == res);

    NRF_LOG_INFO("MAIN: blinky_on_button_release: HSV: %d %d %d", g_data.hsv.h, g_data.hsv.s, g_data.hsv.v);
}

static void blinky_save_data(data_t* data)
{
    NRF_LOG_INFO("MAIN: blinky_save_data");
    ASSERT(NULL != data);

    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.saved_hsv.h=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.h));
    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.hsv.h=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.hsv.h));
    NRF_LOG_INFO("MAIN: ");
    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.saved_hsv.s=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.s));
    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.hsv.s=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.hsv.s));
    NRF_LOG_INFO("MAIN: ");
    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.saved_hsv.v=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.v));
    NRF_LOG_INFO("MAIN: blinky_save_data: g_data.hsv.v=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.hsv.v));
    
    if ((abs(data->saved_hsv.h - data->hsv.h) < 1.f) &&
        (abs(data->saved_hsv.s - data->hsv.s) < 1.f) &&
        (abs(data->saved_hsv.v - data->hsv.v) < 1.f))
        {
            NRF_LOG_INFO("MAIN: blinky_save_data: Nothing to save");
            return;
        }

    bool result = blinky_nvmc_write_data((uint32_t*)&(data->hsv), sizeof(hsv_t));
    NRF_LOG_INFO("MAIN: blinky_save_data: blinky_nvmc_write_data result=%d", result);
    while (!blinky_nvmc_write_done_check())
    {}
    NRF_LOG_INFO("MAIN: blinky_save_data: writing complete");
    data->saved_hsv.h = data->hsv.h;
}

void blinky_on_button_multi_click(void * p_context)
{
    NRF_LOG_INFO("MAIN: blinky_on_button_multi_click");
    ASSERT(NULL!= p_context);
    uint32_t click_cnt = (uint32_t)p_context;

    switch (click_cnt)
    {
        case 1:
        {
            NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: Single click handling...");
            break;
        }
        case 2:
            NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: Double click handling...");
            NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: old state: %s", blinky_state_to_str(g_data.state));
            g_data.state++;
            g_data.state %= T_COUNT;
            blinky_state_to_led(g_data.state);
            NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: new state: %s", blinky_state_to_str(g_data.state));
            if(g_data.state == T_VIEW)
            {
                blinky_save_data(&g_data);
            }
            break;
        case 3:
            NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: Triple click handling...");
            break;
        default:
            break;
    }
}

void blinky_set_led_rgb(rgb_t* rgb)
{
    ASSERT(NULL != rgb);
    blinky_led_pwm_set(BLINKY_LED_R, rgb->r);
    blinky_led_pwm_set(BLINKY_LED_G, rgb->g);
    blinky_led_pwm_set(BLINKY_LED_B, rgb->b);
}

void blinky_360_run(volatile float* value)
{
    ASSERT(NULL != value);
    *value += 1.f;
    if (*value > 360.f)
    {
        *value = 0.f;
    }
}

void blinky_100_run(volatile float* value, volatile bool* up)
{
    ASSERT(NULL != value);
    ASSERT(NULL != up);
    if (*up)
    {
        *value += 1.f;
        if( *value > 100.f)
        {
            *value = 100.f;
            *up = false;
        }
    }
    else
    {
        *value -= 1.f;
        if( *value < 0.f)
        {
            *value = 0.f;
            *up = true;
        }
    }
}
#ifdef USB_CLI_ENABLED
void blinky_on_command(char* s_command)
{
    NRF_LOG_INFO("MAIN: blinky_on_command...");
    command_t t_command;
    blinky_command_process(s_command, &t_command);
    switch (t_command.type)
    {
        case T_RGB:
        {
            rgb2hsv(&t_command.color.rgb, (hsv_t*)&(g_data.hsv));
            blinky_set_led_rgb(&t_command.color.rgb);
            break;
        }
        case T_HSV:
        {
            g_data.hsv = t_command.color.hsv;
            rgb_t rgb = { 0.f };
            hsv2rgb((hsv_t*)&(g_data.hsv), &rgb);
            blinky_set_led_rgb(&rgb);
            break;
        }
        case T_HELP:
        {
            bool ret = blinky_cdc_acm_send_str(
                "CLI to control Nordic PCA10059 board\r\n" \
                "\r\n" \
                "These are common CLI commands used in various situations:\r\n" \
                "\r\n" \
                "  help\t\t\toutput this information.\r\n" \
                "  rgb {r} {g} {b}\tsetup the led 2 color, r - red component [0..100], g - green component [0..100], b - blue component [0..100].\r\n" \
                "  hsv {h} {s} {v}\tsetup the led 2 color, h - hue component [0..360], s - saturation component [0..100], v - value/brightness component [0..100]. \r\n" \
                "  save\t\t\tsave current color of led 2 to nvmc.\r\n" \
                " \r\n");
            if(!ret)
            {
                NRF_LOG_INFO("MAIN: blinky_on_command: Probably sending is to fast, the previous package is in progress");
            }
            break;
        }
        case T_SAVE:
        {
            /* Don't call long operation under interruption, just set the flag */
            g_data.need_save = true;
            break;
        }
        case T_EMPTY:
        {
            bool ret = blinky_cdc_acm_send_str(">\r\n");
            if(!ret)
            {
                NRF_LOG_INFO("MAIN: blinky_on_command: Probably sending is to fast, the previous package is in progress");
            }
            break;
        }
        case T_UNKNOWN:// fall-through
        default:
        {
            bool ret = blinky_cdc_acm_send_str(
                "unknown command or wrong format\r\n" \
                "\r\n" \
                "usage: <command> [args]\r\n" \
                "\r\n" \
                "'help' to see available commands\r\n");
            if(!ret)
            {
                NRF_LOG_INFO("MAIN: blinky_on_command: Probably sending is to fast, the previous package is in progress");
            }
            break;
        }
    }
}
#endif
void app_timer_move_handler(void * p_context)
{
    switch(g_data.state)
    {
        case T_VIEW:
            break;
    
        case T_EDIT_HUE:
            blinky_360_run(&(g_data.hsv.h));
            break;

        case T_EDIT_SATURATION:
            blinky_100_run(&(g_data.hsv.s), &(g_data.move_s_up));
            break;

        case T_EDIT_BRIGHTNESS:
            blinky_100_run(&(g_data.hsv.v), &(g_data.move_v_up));
            break;

        case T_COUNT: // fall-through
        default:
            ASSERT(false);
            break;
    }
 
    rgb_t rgb = { 0.f };
    hsv2rgb((hsv_t*)&(g_data.hsv), &rgb);
    blinky_set_led_rgb(&rgb);
}

void blinky_init(void)
{
    /* Logs init */
    ret_code_t res = NRF_LOG_INIT(NULL);
    UNUSED_VARIABLE(res);
    ASSERT(NRF_SUCCESS == res);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

#ifdef USB_CLI_ENABLED
    /* USB CDC ACM init */
    blinky_cdc_acm_init(blinky_on_command);
#endif
    /* Timers init */
    NRF_LOG_INFO("MAIN: Timers init");
    res = app_timer_init();
    ASSERT(NRF_SUCCESS == res);
    res = app_timer_create(&g_timer_move, APP_TIMER_MODE_REPEATED, app_timer_move_handler);
    ASSERT(NRF_SUCCESS == res);

    /* Leds init */
    NRF_LOG_INFO("MAIN: Leds init");
    blinky_led_soft_init();

    /* HACK to turn off leds that is a result of ASSERT in USB CDC ACM module during init */
    blinky_led_pwm_set(BLINKY_LED_0, 0);
    blinky_led_pwm_set(BLINKY_LED_1, 0);
    blinky_led_pwm_set(BLINKY_LED_2, 0);
    blinky_led_pwm_set(BLINKY_LED_3, 0);

    /* Buttons init */
    NRF_LOG_INFO("MAIN: Buttons init");
    blinky_btns_init(blinky_on_button_hold, blinky_on_button_release, blinky_on_button_multi_click);

    /* NVMC init */
    NRF_LOG_INFO("MAIN: NVMC init");
    blinky_nvmc_init(sizeof(hsv_t));
    
    uint32_t read = blinky_nvmc_read_last_data((uint32_t*)&(g_data.saved_hsv), sizeof(g_data.saved_hsv));
    NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: blinky_nvmc_get_last_data read %u bytes", read);
    if (read > 0)
    {
        NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: g_data.saved_hsv.h=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.h));
        NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: g_data.saved_hsv.v=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.v));
        NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: g_data.saved_hsv.s=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved_hsv.s));
        g_data.hsv = g_data.saved_hsv;
    }

    /* Color init defaults */
    NRF_LOG_INFO("MAIN: Color init defaults");
    rgb_t rgb = {0.f};
    hsv2rgb((hsv_t*)&(g_data.hsv), &rgb);
    blinky_set_led_rgb(&rgb);

    
 }

/* Application main entry.*/
int main(void)
{   
    blinky_init();

    /* Main loop */
    NRF_LOG_INFO("MAIN: Main loop go");
    while (true)
    {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
#ifdef USB_CLI_ENABLED
        blinky_cdc_acm_loop();
        
        if (g_data.need_save)
        {
            blinky_save_data(&g_data);
            g_data.need_save = false;
        }
#endif    


#if !defined(DEBUG_NRF) && !defined(USB_CLI_ENABLED)
        /* Wait for interrapt.
        Incorrect log output and usb data exchange in sleep mode */
        __WFI();
#endif
    }

    return 0;
}