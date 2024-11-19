#include <math.h>

#include "blinky_color.h"

rgb_t hsv2rgb(hsv_t hsv)
{
	float r = 0.f;
    float g = 0.f;
    float b = 0.f;
	
	float h = hsv.h / 360.f;
	float s = hsv.s / 100.f;
	float v = hsv.v / 100.f;
	
	int i = floor(h * 6.f);
	float f = h * 6.f - i;
	float p = v * (1.f - s);
	float q = v * (1.f - f * s);
	float t = v * (1.f - (1.f - f) * s);
	
	switch (i % 6)
    {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	
	rgb_t color;
	color.r = r * 100.f;
	color.g = g * 100.f;
	color.b = b * 100.f;
	
	return color;
}
