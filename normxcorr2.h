#ifndef TEMPLATE_MATCHING_H
#define TEMPLATE_MATCHING_H

typedef struct image {
    int width;
    int height;
    float *data;
} image_t;

image_t normxcorr2(image_t templ, image_t image);

#endif
