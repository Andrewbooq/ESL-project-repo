#include "blinky_command.h"

#include "blinky_log.h"

/* two-dimensional array char arguments[cnt][size_one]
   returns substr_count */
static uint32_t string_split(char* str, const char* delimiter, char* arguments, uint32_t cnt, uint32_t size_one)
{
    ASSERT(NULL != str);
    ASSERT(NULL != delimiter);
    ASSERT(NULL != arguments);

    uint32_t substr_cnt = 0;

    char* next = strtok(str, delimiter);
    while (next != NULL)
    {
        if (substr_cnt < cnt)
        {
            uint32_t capacity = MIN(strlen(next) , size_one);
            strncpy(arguments + (substr_cnt * size_one), next, capacity);
        }
        substr_cnt++;
        next = strtok(NULL, delimiter);
    }
    return substr_cnt;
}


static int32_t clamp(int32_t number, int32_t min_value, int32_t max_value)
{
    if (number > max_value)
    {
        number = max_value;
    }
    else if (number < min_value)
    {
        number = min_value;
    }
    return number;
}

static bool is_it_number(const char* str)
{
    ASSERT(NULL != str);
    if ((NULL != str) && strspn(str, BLINKY_NUMBERS) == strlen(str))
    {
        return true;
    }
    return false;
}

static bool is_it_string(const char* str)
{
    ASSERT(NULL != str);
    if ((NULL != str) && strspn(str, BLINKY_ALPHABET) == strlen(str))
    {
        return true;
    }
    return false;
}

static bool is_it_characters(const char* str)
{
    ASSERT(NULL != str);
    if ((NULL != str) && strspn(str, BLINKY_CHARACTERS) == strlen(str))
    {
        return true;
    }
    return false;
}

void blinky_command_process(char* s_command, command_t* t_command)
{
    NRF_LOG_INFO("CMD: blinky_command_process");
    ASSERT(NULL != s_command);
    ASSERT(NULL != t_command);

    NRF_LOG_INFO("CMD: blinky_command_process: Command len=%u", strlen(s_command));

    char arguments[BLINKY_CMD_MAX_PARAMS][BLINKY_CMD_MAX_PARAM_SIZE] = { 0 };
    uint32_t nargs = string_split(s_command, BLINKY_CMD_DELIMITER, (char*)arguments, BLINKY_CMD_MAX_PARAMS, BLINKY_CMD_MAX_PARAM_SIZE);

    if (nargs == 0)
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Empty command");
        t_command->type = T_EMPTY;
        return;
    }
    else if (!is_it_string(arguments[0]))
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Unknown command");
        t_command->type = T_UNKNOWN;
        return;
    }
    
    char* pend = NULL;
    if (0 == strcmp(arguments[0], "rgb")
        && nargs == 4
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3]))
    {
        uint8_t r = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t g = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t b = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command rgb, %u %u %u", r, g, b);

        t_command->type = T_RGB;

        t_command->color_data.rgb.r = (float)r;
        t_command->color_data.rgb.g = (float)g;
        t_command->color_data.rgb.b = (float)b;
    }
    else if (0 == strcmp(arguments[0], "hsv")
        && nargs == 4
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3]))
    {
        uint8_t h = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_HUE);
        uint8_t s = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t v = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command hsv, %u %u %u", h, s, v);

        t_command->type = T_HSV;
        t_command->color_data.hsv.h = (float)h;
        t_command->color_data.hsv.s = (float)s;
        t_command->color_data.hsv.v = (float)v;
    }
    else if (0 == strcmp(arguments[0], "save")
        && nargs == 1)
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command save");
        t_command->type = T_SAVE;
    }
    else if (0 == strcmp(arguments[0], "help")
        && nargs == 1)
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command help");
        t_command->type = T_HELP;
    }
    else if (0 == strcmp(arguments[0], "add_rgb_color")
        && nargs == 5
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3])
        && is_it_characters(arguments[4]))
    {
        if(strlen(arguments[4]) >= BLINKY_CMD_MAX_PARAM_SIZE)
        {
            NRF_LOG_INFO("CMD: blinky_command_handler: color name too long");
            t_command->type = T_UNKNOWN;
            return;
        }

        uint8_t r = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t g = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t b = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command add_rgb_color, %u %u %u", r, g, b);

        t_command->type = T_RGBNAME;
        t_command->color_data.rgbname_pair.rgb.r = (float)r;
        t_command->color_data.rgbname_pair.rgb.g = (float)g;
        t_command->color_data.rgbname_pair.rgb.b = (float)b;
        strncpy(t_command->color_data.rgbname_pair.name, arguments[4], BLINKY_CMD_MAX_PARAM_SIZE);
    }
    else if (0 == strcmp(arguments[0], "add_hsv_color")
        && nargs == 5
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3])
        && is_it_characters(arguments[4]))
    {
        if(strlen(arguments[4]) >= BLINKY_CMD_MAX_PARAM_SIZE)
        {
            NRF_LOG_INFO("CMD: blinky_command_handler: color name too long");
            t_command->type = T_UNKNOWN;
            return;
        }

        uint8_t h = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_HUE);
        uint8_t s = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);
        uint8_t v = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_COMPONENT, BLINKY_MAX_RGBSV);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command add_hsv_color, %u %u %u", h, s, v);

        t_command->type = T_HSVNAME;
        t_command->color_data.hsvname_pair.hsv.h = (float)h;
        t_command->color_data.hsvname_pair.hsv.s = (float)s;
        t_command->color_data.hsvname_pair.hsv.v = (float)v;
        strncpy(t_command->color_data.hsvname_pair.name, arguments[4], BLINKY_CMD_MAX_PARAM_SIZE);
    }
    else if (0 == strcmp(arguments[0], "add_current_color")
        && nargs == 2
        && is_it_characters(arguments[1]))
    {
        if(strlen(arguments[1]) >= BLINKY_CMD_MAX_PARAM_SIZE)
        {
            NRF_LOG_INFO("CMD: blinky_command_handler: color name too long");
            t_command->type = T_UNKNOWN;
            return;
        }

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command add_current_color");

        t_command->type = T_ADD_CURRENT_COLOR;
        strncpy(t_command->color_data.name, arguments[1], BLINKY_CMD_MAX_PARAM_SIZE);
    }
    else if (0 == strcmp(arguments[0], "del_color")
        && nargs == 2
        && is_it_characters(arguments[1]))
    {
        if(strlen(arguments[1]) >= BLINKY_CMD_MAX_PARAM_SIZE)
        {
            NRF_LOG_INFO("CMD: blinky_command_handler: color name too long");
            t_command->type = T_UNKNOWN;
            return;
        }

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command del_color");

        t_command->type = T_DELETE_COLOR;
        strncpy(t_command->color_data.name, arguments[1], BLINKY_CMD_MAX_PARAM_SIZE);
    }
    else if (0 == strcmp(arguments[0], "apply_color")
        && nargs == 2
        && is_it_characters(arguments[1]))
    {
        if(strlen(arguments[1]) >= BLINKY_CMD_MAX_PARAM_SIZE)
        {
            NRF_LOG_INFO("CMD: blinky_command_handler: color name too long");
            t_command->type = T_UNKNOWN;
            return;
        }

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command apply_color");

        t_command->type = T_APPLY_COLOR;
        strncpy(t_command->color_data.name, arguments[1], BLINKY_CMD_MAX_PARAM_SIZE);
    }
    else if (0 == strcmp(arguments[0], "list_colors")
        && nargs == 1)
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command list_colors");

        t_command->type = T_LIST_COLOR;
    }
    else
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Unknown command");
        t_command->type = T_UNKNOWN;
    }
}