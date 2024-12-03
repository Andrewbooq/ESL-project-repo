#include <math.h>

#include "sdk_common.h"
#include "blinky_color.h"

void hsv2rgb(hsv_t* hsv, rgb_t* rgb)
{
    ASSERT(NULL != rgb);
    ASSERT(NULL != hsv);

    float r = 0.f;
    float g = 0.f;
    float b = 0.f;

    float h = hsv->h / 360.f;
    float s = hsv->s / 100.f;
    float v = hsv->v / 100.f;

    int i = floor(h * 6.f);
    float f = h * 6.f - i;
    float p = v * (1.f - s);
    float q = v * (1.f - f * s);
    float t = v * (1.f - (1.f - f) * s);

    switch (i % 6)
    {
        case 0:
            r = v, g = t, b = p;
            break;
        case 1:
            r = q, g = v, b = p;
            break;
        case 2:
            r = p, g = v, b = t;
            break;
        case 3:
            r = p, g = q, b = v;
            break;
        case 4:
            r = t, g = p, b = v;
            break;
        case 5:
            r = v, g = p, b = q;
            break;
        default:
            break;
    }

    rgb->r = r * 100.f;
    rgb->g = g * 100.f;
    rgb->b = b * 100.f;
}

void rgb2hsv(rgb_t* rgb, hsv_t* hsv)
{
    ASSERT(NULL != rgb);
    ASSERT(NULL != hsv);

    float h = 0.f;
    float s = 0.f;
    float v = 0.f;

    float r = rgb->r / 100.f;
    float g = rgb->g / 100.f;
    float b = rgb->b / 100.f;

    float c_max = fmax(fmax(r, g), b);
    float c_min = fmin(fmin(r, g), b);
    float delta = c_max - c_min;

    if (delta > 0.f)
    {
        if (c_max == r)
        {
            h = 60.f * (fmod(((g - b) / delta), 6.f));
        } else if (c_max == g)
        {
            h = 60.f * (((b - r) / delta) + 2.f);
        } else if (c_max == b)
        {
            h = 60.f * (((r - g) / delta) + 4.f);
        }
    
        if (c_max > 0.f)
        {
            s = delta / c_max;
        } else
        {
            s = 0.f;
        }
    
        v = c_max;
    } else
    {
        h = 0.f;
        s = 0.f;
        v = c_max;
    }
  
    if (h < 0.f)
    {
        h += 360.f;
    }

    hsv->h = h;
    hsv->s = s * 100.f;
    hsv->v = v * 100.f;
}
