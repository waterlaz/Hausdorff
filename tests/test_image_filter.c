#include "image.h"
#include "filters.h"

int main(int argc, char** argv){
    init_image_lib();
    image_t* img = image_load(argv[1]);
    image_t* res = filter_median(img, 1);
    image_save(res, argv[2]);
}
