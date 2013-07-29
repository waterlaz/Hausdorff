#include "image.h"

int main(int argc, char** argv){
    init_image_lib();
    image_t* img = image_load(argv[1]);
}
