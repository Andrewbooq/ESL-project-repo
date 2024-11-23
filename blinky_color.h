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

void hsv2rgb(hsv_t* hsv, rgb_t* rgb);

#endif //BLINKY_COLOR_H__
