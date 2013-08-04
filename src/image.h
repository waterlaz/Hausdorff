#ifndef __IMAGE_H_
#define __IMAGE_H_ 

typedef struct _pixel_t {
    int r, g, b;
} pixel_t;

typedef struct _image_t {
    int w;
    int h;
    pixel_t** img;
} image_t;

image_t* image_create(int w, int h);

void init_image_lib();

image_t* image_load(char *s);

void image_draw_line(image_t* img, int x1, int y1, int x2, int y2, pixel_t color);

#endif
