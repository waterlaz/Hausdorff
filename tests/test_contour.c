#include "image.h"
#include "contour.h"

int main(int argc, char** argv){
    init_image_lib();
    image_t* img = image_load(argv[1]);
    int* level = (int*)malloc(sizeof(int)*5);
    level[0]=50;
    level[1]=100;
    level[2]=150;
    level[3]=200;
    level[4]=250;
    find_contours(img, 5, level);
    image_save(img, "dump.bmp");
}
