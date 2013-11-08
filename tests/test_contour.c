#include "image.h"
#include "contour.h"
#include <stdlib.h>

int main(int argc, char** argv){
    init_image_lib();
    image_t* img = image_load(argv[1]);
    int* level = (int*)malloc(sizeof(int)*255);
    int i;
    for(i=0; i<=255; i++)
        level[i]=i;
    contour_set_t* c = find_contours(img, 255, level);
    image_save(img, "dump.bmp");
    image_free(img);
    free(level);
}
