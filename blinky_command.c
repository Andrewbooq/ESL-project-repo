#include "blinky_command.h"

#include "blinky_log.h"

#define BLINKY_MAX_PARAMS       4
#define BLINKY_MAX_PARAM_SIZE   20
#define BLINKY_DELIMITER        " "
#define BLINKY_NUMBERS          "+-0123456789"
#define BLINKY_ALPHABET         "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define BLINKY_MIN_VALUE        0
#define BLINKY_MAX_RGB          100
#define BLINKY_MAX_HUE          360

uint32_t string_split(char* str, const char* delimiter, char* arguments, uint32_t cnt, uint32_t size_one)
{
    unsigned int uParam = 0;

    char* next = strtok(str, delimiter);
    while (next != NULL)
    {
        if (uParam < cnt)
        {
            uint32_t capacity = MIN(strlen(next) , size_one);
            strncpy(arguments + (uParam * size_one), next, capacity);
        }
        uParam++;
        next = strtok(NULL, delimiter);
    }
    return uParam;
}


int32_t clamp(int32_t number, int32_t min_value, int32_t max_value)
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

bool is_it_number(const char* str)
{
    if ((NULL != str) && strspn(str, BLINKY_NUMBERS) == strlen(str))
    {
        return true;
    }
    return false;
}

bool is_it_string(const char* str)
{
    if ((NULL != str) && strspn(str, BLINKY_ALPHABET) == strlen(str))
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

    char arguments[BLINKY_MAX_PARAMS][BLINKY_MAX_PARAM_SIZE] = { 0 };
    uint32_t nargs = string_split(s_command, BLINKY_DELIMITER, (char*)arguments, BLINKY_MAX_PARAMS, BLINKY_MAX_PARAM_SIZE);
	
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
    
    if (0 == strcmp(arguments[0], "rgb")
        && nargs == 4
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3]))
    {
        char* pend = NULL;
        
        uint8_t r = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_RGB);
        uint8_t g = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_RGB);
        uint8_t b = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_RGB);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command rgb=%u %u %u", r, g, b);

        t_command->type = T_RGB;
        t_command->color.rgb.r = (float)r;
        t_command->color.rgb.g = (float)g;
        t_command->color.rgb.b = (float)b;
    }
    else if (0 == strcmp(arguments[0], "hsv")
        && nargs == 4
        && is_it_number(arguments[1])
        && is_it_number(arguments[2])
        && is_it_number(arguments[3]))
    {
        char* pend = NULL;
        
        uint8_t h = clamp(strtol(arguments[1], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_HUE);
        uint8_t s = clamp(strtol(arguments[2], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_RGB);
        uint8_t v = clamp(strtol(arguments[3], &pend, 10), BLINKY_MIN_VALUE, BLINKY_MAX_RGB);

        NRF_LOG_INFO("CMD: blinky_command_handler: Recognized command hsv=%u %u %u", h, s, v);

        t_command->type = T_HSV;
        t_command->color.hsv.h = (float)h;
        t_command->color.hsv.s = (float)s;
        t_command->color.hsv.v = (float)v;
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
    else
    {
        NRF_LOG_INFO("CMD: blinky_command_handler: Unknown command");
        t_command->type = T_UNKNOWN;
    }
    
}