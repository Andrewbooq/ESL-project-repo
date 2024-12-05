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
    .current = 
    {
        .hsv = 
        {
            /* DEVICE_ID=6584
            Last digits: 84
            Hue: 84% => 360 * 0.84 = 302° */
            .h = ((BLINKY_SN_C * 10.f) + BLINKY_SN_D)  * 360.f / 100.f,
            .s = 100.f,
            .v = 100.f
        },
    },
    .saved = 
    {
        .hsv = 
        {
            .h = 50.f,
            .s = 50.f,
            .v = 50.f 
        }
    },
};

STATIC_ASSERT(sizeof(g_data.saved) % sizeof(uint32_t) == 0, "struct must be aligned to 32 bit word");

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

    NRF_LOG_INFO("MAIN: blinky_on_button_release: HSV: %d %d %d", g_data.current.hsv.h, g_data.current.hsv.s, g_data.current.hsv.v);
}
 
static bool are_hsv_equal(hsv_t* first, hsv_t* second)
{
    ASSERT(NULL != first);
    ASSERT(NULL != second);

    if ((abs(first->h - second->h) < 1.f) &&
        (abs(first->s - second->s) < 1.f) &&
        (abs(first->v - second->v) < 1.f))
    {
        return true;
    }
    return false;
}

static bool blinky_are_current_equal(color_data_t* first, color_data_t* second)
{
    ASSERT(NULL != first);
    ASSERT(NULL != second);
    
    if (!are_hsv_equal(&(first->hsv), &(second->hsv)))
    {
        return false;
    }

    for (uint32_t i = 0; i < BLINKY_SAVED_COLOR_CNT; ++i)
    {
        if( 0 != strcmp(first->hsvname_pair_array[i].name, second->hsvname_pair_array[i].name) ||
            !are_hsv_equal(&(first->hsvname_pair_array[i].hsv), &(second->hsvname_pair_array[i].hsv)))
        {
            return false;
        }
    }

    return true;
}

static void blinky_save_data(data_t* data)
{
    NRF_LOG_INFO("MAIN: blinky_save_data");
    ASSERT(NULL != data);

    if (blinky_are_current_equal((color_data_t*)&(data->current), &(data->saved)))
    {
        NRF_LOG_INFO("MAIN: blinky_save_data: Nothing to save");
        return;
    }

    bool result = blinky_nvmc_write_data((uint32_t*)&(data->current), sizeof(data->current));
    NRF_LOG_INFO("MAIN: blinky_save_data: blinky_nvmc_write_data result=%d", result);
    if(result)
    {
        while (!blinky_nvmc_write_done_check())
        {}
    }
    NRF_LOG_INFO("MAIN: blinky_save_data: writing complete");
    memcpy(&(g_data.saved), (void*)&(g_data.current), sizeof(g_data.saved));
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

static void blinky_set_led_rgb(rgb_t* rgb)
{
    ASSERT(NULL != rgb);
    blinky_led_pwm_set(BLINKY_LED_R, rgb->r);
    blinky_led_pwm_set(BLINKY_LED_G, rgb->g);
    blinky_led_pwm_set(BLINKY_LED_B, rgb->b);
}

static void blinky_360_run(volatile float* value)
{
    ASSERT(NULL != value);
    *value += 1.f;
    if (*value > 360.f)
    {
        *value = 0.f;
    }
}

static void blinky_100_run(volatile float* value, volatile bool* up)
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

static int32_t blinky_get_empty_name_index(hsvname_pair_t* hsvname_pair, uint32_t size)
{
    ASSERT(NULL != hsvname_pair);
    for(uint32_t i = 0; i < size; ++i)
    {
        if (0 == strlen(hsvname_pair[i].name))
        {
            return i;
        }
    }
    return -1;    
}

static int32_t blinky_get_index_by_name(hsvname_pair_t* hsvname_pair, uint32_t size, char* name)
{
    ASSERT(NULL != hsvname_pair);
    ASSERT(NULL != name);
    for(uint32_t i = 0; i < BLINKY_SAVED_COLOR_CNT; ++i)
    {
        if (0 == strcmp(hsvname_pair[i].name, name))
        {
            return i;
        }
    }
    return -1;    
}

void blinky_on_command(char* s_command)
{
    NRF_LOG_INFO("MAIN: blinky_on_command...");
    command_t t_command;
    blinky_command_process(s_command, &t_command);
    bool ret = true;
    switch (t_command.type)
    {
        case T_EMPTY:
        {
            ret = blinky_cdc_acm_send_str(BLINKY_WELCOME_STRING);
            break;
        }
        case T_RGB:
        {
            rgb2hsv(&t_command.color_data.rgb, (hsv_t*)&(g_data.current.hsv));
            blinky_set_led_rgb(&t_command.color_data.rgb);
            break;
        }
        case T_RGBNAME:
        {
            int32_t index = blinky_get_index_by_name((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT, t_command.color_data.rgbname_pair.name);
            if (index >= 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_ITEM_EXISTS_STRING);
                break;
            }
            index = blinky_get_empty_name_index((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT);
            if (index < 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_FULL_STORAGE_STRING);
            }
            else
            {
                hsv_t hsv = {0.f};
                rgb2hsv(&t_command.color_data.rgbname_pair.rgb, &hsv);
                g_data.current.hsvname_pair_array[index].hsv = hsv;
                strncpy((char*)g_data.current.hsvname_pair_array[index].name, t_command.color_data.rgbname_pair.name, BLINKY_COLOR_NAME_SIZE);
            }
            break;
        }
        case T_HSV:
        {
            g_data.current.hsv = t_command.color_data.hsv;
            rgb_t rgb = { 0.f };
            hsv2rgb((hsv_t*)&(g_data.current.hsv), &rgb);
            blinky_set_led_rgb(&rgb);
            break;
        }
        case T_HSVNAME:
        {
            int32_t index = blinky_get_index_by_name((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT, t_command.color_data.hsvname_pair.name);
            if (index >= 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_ITEM_EXISTS_STRING);
                break;
            }
            index = blinky_get_empty_name_index((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT);
            if (index < 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_FULL_STORAGE_STRING);
            }
            else
            {
                g_data.current.hsvname_pair_array[index].hsv = t_command.color_data.hsvname_pair.hsv;
                strncpy((char*)g_data.current.hsvname_pair_array[index].name, t_command.color_data.hsvname_pair.name, BLINKY_COLOR_NAME_SIZE);
            }
            break;
        }
        case T_HELP: 
        {
            ret = blinky_cdc_acm_send_str(BLINKY_HELP_STRING);
            break;
        }
        case T_SAVE:
        {
            /* Don't call long operation under interruption, just set the flag */
            g_data.need_save = true;
            break;
        }
        case T_ADD_CURRENT_COLOR:
        {
            int32_t index = blinky_get_index_by_name((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT, t_command.color_data.name);
            if (index >= 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_ITEM_EXISTS_STRING);
                break;
            }
            index = blinky_get_empty_name_index((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT);
            if (index < 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_FULL_STORAGE_STRING);
            }
            else
            {
                g_data.current.hsvname_pair_array[index].hsv = g_data.current.hsv;
                strncpy((char*)g_data.current.hsvname_pair_array[index].name, t_command.color_data.name, BLINKY_COLOR_NAME_SIZE);
            }
            break;
        }
        case T_DELETE_COLOR:
        {
            int32_t index = blinky_get_index_by_name((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT, t_command.color_data.name);
            if (index < 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_NO_ITEM_STRING);
            }
            else
            {
                memset((void*)&g_data.current.hsvname_pair_array[index], 0, sizeof(hsvname_pair_t));
            }
            break;
        }
        case T_APPLY_COLOR:
        {
            int32_t index = blinky_get_index_by_name((hsvname_pair_t*)g_data.current.hsvname_pair_array, BLINKY_SAVED_COLOR_CNT, t_command.color_data.name);
            if (index < 0)
            {
                ret = blinky_cdc_acm_send_str(BLINKY_NO_ITEM_STRING);
            }
            else
            {
                g_data.current.hsv = g_data.current.hsvname_pair_array[index].hsv;
                rgb_t rgb = { 0.f };
                hsv2rgb((hsv_t*)&(g_data.current.hsv), &rgb);
                blinky_set_led_rgb(&rgb);
            }
            break;
        }
        case T_LIST_COLOR:
        {
            char name_list [BLINKY_SAVED_COLOR_CNT * (BLINKY_COLOR_NAME_SIZE + 2)] = { 0 };
            for(uint32_t i = 0; i < BLINKY_SAVED_COLOR_CNT; ++i)
            {
                if (0 != strlen((char*)g_data.current.hsvname_pair_array[i].name))
                {
                    strncat(name_list, (char*)g_data.current.hsvname_pair_array[i].name, BLINKY_COLOR_NAME_SIZE);
                    strncat(name_list, "\r\n", 2);
                }
            }
            ret = blinky_cdc_acm_send_str(name_list);
            break;
        }
        case T_UNKNOWN:// fall-through
        default:
        {
            ret = blinky_cdc_acm_send_str(BLINKY_UNKNOW_STRING);
            break;
        }
    }

    if(!ret)
    {
        NRF_LOG_INFO("MAIN: blinky_on_command: Probably sending is to fast, the previous package is in progress");
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
            blinky_360_run(&(g_data.current.hsv.h));
            break;

        case T_EDIT_SATURATION:
            blinky_100_run(&(g_data.current.hsv.s), &(g_data.move_s_up));
            break;

        case T_EDIT_BRIGHTNESS:
            blinky_100_run(&(g_data.current.hsv.v), &(g_data.move_v_up));
            break;

        case T_COUNT: // fall-through
        default:
            ASSERT(false);
            break;
    }
 
    rgb_t rgb = { 0.f };
    hsv2rgb((hsv_t*)&(g_data.current.hsv), &rgb);
    blinky_set_led_rgb(&rgb);
}

void blinky_init(void)
{
    memset((void*)g_data.current.hsvname_pair_array, 0, sizeof(g_data.current.hsvname_pair_array));
    memset((void*)g_data.saved.hsvname_pair_array, 0, sizeof(g_data.saved.hsvname_pair_array));

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
    ASSERT(sizeof(g_data.saved) == sizeof(g_data.current))
    blinky_nvmc_init(sizeof(g_data.saved));
    
    uint32_t read = blinky_nvmc_read_last_data((uint32_t*)&(g_data.saved), sizeof(g_data.saved));
    NRF_LOG_INFO("MAIN: blinky_on_button_multi_click: blinky_nvmc_get_last_data read %u bytes", read);
    if (read > 0)
    {
        NRF_LOG_INFO("MAIN: saved.hsv.h=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsv.h));
        NRF_LOG_INFO("MAIN: saved.hsv.v=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsv.v));
        NRF_LOG_INFO("MAIN: saved.hsv.s=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsv.s));
        for (uint32_t i = 0; i < BLINKY_SAVED_COLOR_CNT; ++i)
        {
            NRF_LOG_INFO("MAIN: hsvname_pair.name=%s", g_data.saved.hsvname_pair_array[i].name);
            NRF_LOG_INFO("MAIN: hsvname_pair.hsv.h=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsvname_pair_array[i].hsv.h));
            NRF_LOG_INFO("MAIN: hsvname_pair.hsv.v=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsvname_pair_array[i].hsv.v));
            NRF_LOG_INFO("MAIN: hsvname_pair.hsv.s=" NRF_LOG_FLOAT_MARKER, NRF_LOG_FLOAT(g_data.saved.hsvname_pair_array[i].hsv.s));
        }
        memcpy((void*)&(g_data.current), &(g_data.saved), sizeof(g_data.saved));
    }

    /* Color init defaults */
    NRF_LOG_INFO("MAIN: Color init defaults");
    rgb_t rgb = {0.f};
    hsv2rgb((hsv_t*)&(g_data.current.hsv), &rgb);
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