#ifndef BLINKY_TYPES_H__
#define BLINKY_TYPES_H__

#include <stdbool.h>
#include "nrfx_common.h"

#define QUOTE(x) #x
#define STR(x) QUOTE(x)

#define BLINKY_CMD_MAX_PARAMS       5
#define BLINKY_CMD_MAX_PARAM_SIZE   15
#define BLINKY_MIN_COMPONENT        0
#define BLINKY_MAX_RGBSV            100
#define BLINKY_MAX_HUE              360
#define BLINKY_READ_BUF_SIZE        1
#define BLINKY_COMMAND_BUF_SIZE     64
#define BLINKY_SEND_BUF_SIZE        2048
#define BLINKY_COLOR_NAME_SIZE      BLINKY_CMD_MAX_PARAM_SIZE
#define BLINKY_SAVED_COLOR_CNT      10
#define BLINKY_CMD_DELIMITER        " "
#define BLINKY_NUMBERS              "+-0123456789"
#define BLINKY_ALPHABET             "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
#define BLINKY_CHARACTERS           "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"

#define BLINKY_HELP_STRING          "CLI to control Nordic PCA10059 board\r\n" \
                                    "\r\n" \
                                    "These are common CLI commands used in various situations:\r\n" \
                                    "\r\n" \
                                    "*  help - Output this information\r\n" \
                                    "*  rgb <r> <g> <b> - Setup the led 2 color, r - red component [0..100], g - green component [0..100], b - blue component [0..100]\r\n" \
                                    "*  hsv <h> <s> <v> - Setup the led 2 color, h - hue component [0..360], s - saturation component [0..100], v - value/brightness component [0..100]\r\n" \
                                    "*  add_rgb_color <r> <g> <b> <color_name> - Add rgb color with name to local storage in RAM, \r\n" \
                                    "        r - red component [0..100], g - green component [0..100], b - blue component [0..100]\r\n" \
                                    "        color_name - name of color to setup color by name [numbers, letters, _], len="STR(BLINKY_COLOR_NAME_SIZE)"\r\n" \
                                    "*  add_hsv_color <h> <s> <v> <color_name> - Add hsv color with name to local storage in RAM, \r\n" \
                                    "        h - hue component [0..360], s - saturation component [0..100], v - value/brightness component [0..100]\r\n" \
                                    "        color_name - name of color to setup color by name [numbers, letters, _], len="STR(BLINKY_COLOR_NAME_SIZE)"\r\n" \
                                    "*  add_current_color <color_name> - Add current color of led 2 with name to local storage in RAM,\r\n" \
                                    "        color_name - name of color to setup color by name [numbers, letters, _], len="STR(BLINKY_COLOR_NAME_SIZE)"\r\n" \
                                    "*  del_color <color_name> - Delete the color by name from local storage in RAM\r\n" \
                                    "*  apply_color <color_name> - Apply the color from local storage by name to led 2\r\n" \
                                    "*  list_colors - Show a list of colors from local storage in RAM\r\n" \
                                    "*  save - Save data to NVMC. Data includes the current color of led 2, local storage of pairs [rgb|hsv color]-[string name]\r\n" \
                                    " \r\n"

#define BLINKY_UNKNOW_STRING        "Unknown command or wrong format\r\n" \
                                    "\r\n" \
                                    "usage: <command> [args]\r\n" \
                                    "\r\n" \
                                    "'help' to see available commands\r\n"

#define BLINKY_WELCOME_STRING       "> "

#define BLINKY_FULL_STORAGE_STRING  "The local storage is full. Delete color before adding\r\n"
#define BLINKY_NO_ITEM_STRING       "Item not found\r\n"
#define BLINKY_ITEM_EXISTS_STRING   "Item already exists\r\n"

/* Colors */
typedef struct
{
    float r;
    float g;
    float b;
} rgb_t;

typedef struct
{
    float h;
    float s;
    float v;
} hsv_t;

typedef struct
{
    rgb_t rgb;
    char name[BLINKY_COLOR_NAME_SIZE];

} rgbname_pair_t;

typedef struct
{
    hsv_t hsv;
    char name[BLINKY_COLOR_NAME_SIZE];

} hsvname_pair_t;


/* State machine for main behavior */
typedef enum
{
    T_VIEW,
    T_EDIT_HUE,
    T_EDIT_SATURATION,
    T_EDIT_BRIGHTNESS,
    T_COUNT
} state_t;

/* Color data to save on NVMC and keep in RAM, a part of main data */
typedef struct
{
    hsv_t hsv;
    hsvname_pair_t hsvname_pair_array[BLINKY_SAVED_COLOR_CNT];
} color_data_t;

/* Main data */
typedef struct 
{
    volatile state_t state;
    volatile bool move_s_up;
    volatile bool move_v_up;
    volatile bool need_save;
    volatile color_data_t current;
    color_data_t saved;
} data_t;

/* Command types */
typedef enum
{
    T_EMPTY,
    T_RGB,
    T_RGBNAME,
    T_HSV,
    T_HSVNAME,
    T_HELP,
    T_SAVE,
    T_ADD_CURRENT_COLOR,
    T_DELETE_COLOR,
    T_APPLY_COLOR,
    T_LIST_COLOR,
    T_UNKNOWN,
} command_type_t;

typedef union
{
    rgb_t rgb;
    hsv_t hsv;
    rgbname_pair_t rgbname_pair;
    hsvname_pair_t hsvname_pair;
    char name[BLINKY_COLOR_NAME_SIZE];
} command_color_data_t;

typedef struct
{
    command_type_t type;
    command_color_data_t color_data;
} command_t;

/* Button clicks callback */
typedef void (*click_cb_t)(void * p_context);

/* USB CDC ACM command callback */
typedef void (*command_cb_t)(char* s_command);

/* USB CDC ACM data*/
typedef struct
{
    char rx_buffer[BLINKY_READ_BUF_SIZE];
    char tx_buffer[BLINKY_SEND_BUF_SIZE];
    char command_buffer[BLINKY_COMMAND_BUF_SIZE];
    volatile uint32_t cmd_index;
    command_cb_t on_command;
    volatile bool need_send;
    volatile uint32_t sent;
    volatile uint32_t want_to_send;
} cdc_acm_t;

/* NVMC data */
typedef struct
{
    uint32_t block_size;
} header_t;

typedef struct
{
    bool need_to_erase_page0;
    uint32_t* last_addr;
    uint32_t writable_block_size;
} nvmc_t;

/* Soft led data */
typedef struct
{
    volatile uint8_t duty_cycle;
    volatile int8_t dark_step;
    volatile bool up;
} led_t;

/* Button data*/
typedef struct
{
    volatile bool timer_multiclick_in_progress;
    volatile uint32_t click_cnt;
    click_cb_t on_button_hold;
    click_cb_t on_button_release;
    click_cb_t on_button_multi_click;
} btn_t;


#endif //BLINKY_TYPES_H__