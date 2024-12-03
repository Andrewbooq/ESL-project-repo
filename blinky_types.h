#ifndef BLINKY_TYPES_H__
#define BLINKY_TYPES_H__

#include <stdbool.h>
//#include "nordic_common.h"
#include "nrfx_common.h"

#define BLINKY_READ_SIZE 1
#define BLINKY_COMMAND_SIZE 64
#define BLINKY_SEND_SIZE 1024

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

/* State machine for main behavior */
typedef enum
{
    T_VIEW,
    T_EDIT_HUE,
    T_EDIT_SATURATION,
    T_EDIT_BRIGHTNESS,
    T_COUNT
} state_t;

/* Main data */
typedef struct 
{
    volatile state_t state;
    volatile bool move_s_up;
    volatile bool move_v_up;
    volatile bool need_save;
    volatile hsv_t hsv;
    hsv_t saved_hsv;
} data_t;

/* Command types */

typedef enum
{
    T_EMPTY,
    T_RGB,
    T_HSV,
    T_HELP,
    T_SAVE,
    T_UNKNOWN,
} command_type_t;

typedef union
{
    rgb_t rgb;
    hsv_t hsv;
} color_t;

typedef struct
{
    command_type_t type;
    color_t color;
} command_t;

/* Button clicks callback */
typedef void (*click_cb_t)(void * p_context);

/* USB CDC ACM command callback */
typedef void (*command_cb_t)(char* s_command);

typedef struct
{
    char rx_buffer[BLINKY_READ_SIZE];
    char tx_buffer[BLINKY_SEND_SIZE];
    char command_buffer[BLINKY_COMMAND_SIZE];
    volatile uint32_t cmd_index;
    command_cb_t on_command;
    volatile bool need_send;
    volatile uint32_t sent;
    volatile uint32_t want_to_send;
} cdc_acm_t;

#endif //BLINKY_TYPES_H__