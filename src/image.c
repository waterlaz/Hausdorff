#include "image.h"

image_t* image_create(int w, int h){
    image_t image;
    image.w = w;
    image.h = h;
    image.img = (pixel_t**)malloc(sizeof(pixel_t*)*w);
}

