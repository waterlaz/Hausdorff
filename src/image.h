#ifndef __IMAGE_H_
#define __IMAGE_H_ 

typedef struct _pixel_t {
    int r, g, b;
} pixel_t;

typedef struct _image_t {
    int w;
    int h;
    pixel_t** img;
}

image_t* image_create(int w, int h);

#endif
