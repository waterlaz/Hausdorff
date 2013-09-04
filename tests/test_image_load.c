#include "image.h"

int main(int argc, char** argv){
    init_image_lib();
    image_t* img = image_load(argv[1]);
    int w = img->w;
    int h = img->h;
    int i;
    pixel_t red;
    red.r = 255;red.g = 0;red.b = 0;
    image_draw_line(img, 0, 0, w/2, h/2, red);
    for(i=0;i<w;i++) img->img[i][h/2].r = 255;
    for(i=0;i<h;i++) img->img[w/2][i].r = 255;
    image_save(img, argv[2]);
}
