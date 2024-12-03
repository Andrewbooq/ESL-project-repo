#ifndef BLINKY_COLOR_H__
#define BLINKY_COLOR_H__

#include "blinky_types.h"

void hsv2rgb(hsv_t* hsv, rgb_t* rgb);
void rgb2hsv(rgb_t* rgb, hsv_t* hsv);

#endif //BLINKY_COLOR_H__
