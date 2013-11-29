#include "image.h"
#include "contour.h"
#include <stdlib.h>

pixel_t red;
pixel_t blue;

void draw(image_t* img, contour_set_t* s){
    if(s->node->is_bright) draw_contour(img, s->node, red);
                    else   draw_contour(img, s->node, blue);

    int i;
    for(i=0; i<s->n; i++){
        draw(img, s->children[i]);
    }
        
}

int main(int argc, char** argv){
    red.r=255;
    red.g=0;
    red.b=0;
    blue.b=255;
    blue.g=0;
    blue.r=0;
    init_image_lib();
    image_t* img = image_load(argv[1]);
    int* level = (int*)malloc(sizeof(int)*256);
    int i;
    for(i=0; i<=255; i++)
        level[i]=i*10;
    contour_set_t* c = find_contours(img, 25, level);
    draw(img, c);
    image_save(img, "dump.bmp");
    image_free(img);
    free_contour_set(c);
    free(level);
}
