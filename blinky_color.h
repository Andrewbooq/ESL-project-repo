#ifndef BLINKY_COLOR_H__
#define BLINKY_COLOR_H__

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

rgb_t hsv2rgb(hsv_t hsv);

#endif //BLINKY_COLOR_H__
