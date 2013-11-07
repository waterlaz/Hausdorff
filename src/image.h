#ifndef __IMAGE_H_
#define __IMAGE_H_ 

#define MAX_COLOR 255

typedef struct _pixel_t {
    int r, g, b;
} pixel_t;

pixel_t pixel_negative(pixel_t pixel);


typedef struct _image_t {
    int w;
    int h;
    pixel_t** img;
} image_t;

image_t* image_create(int w, int h);

void image_free(image_t* img);

image_t* image_copy(image_t* img);

image_t* image_negative(image_t* img);

void init_image_lib();

image_t* image_load(char *s);

static inline int pixel_diff(pixel_t* a, pixel_t* b){
	return (a->r - b->r)*(a->r - b->r) + (a->g - b->g)*(a->g - b->g) + (a->b - b->b)*(a->b - b->b);
}

int image_save(image_t* img, char *s);

void image_draw_line(image_t* img, int x1, int y1, int x2, int y2, pixel_t color);

#endif
