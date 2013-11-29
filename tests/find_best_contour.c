#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "contour.h"
#include "array2d.h"
#include "hausdorff.h"


contour_t* best_contour(contour_set_t* cs, contour_t* c){
}

int main(int argc, char** argv){
    if(argc<4){
        printf("usage %s <input image> <contour> <output image>\n", argv[0]);
        return 0;
    }

    pixel_t red;
    red.r=255;
    red.g=0;
    red.b=0;


    init_image_lib();
    image_t* img = image_load(argv[1]);
    int* level = (int*)malloc(sizeof(int)*256);
    int i;
    for(i=0; i<=25; i++)
        level[i]=i*10;
    contour_set_t* cs = find_contours(img, 26, level);
    contour_t* c = read_contour(argv[2]);
    
    int n;
    contour_t** lc = list_contour_set(cs, &n);

    contour_t** new_lc;
    int new_n;
    
    FILTER(lc, n, new_lc, new_n, i){
        if(*lc[i]->meta.area>100) ACCEPT;
    }
   
    free(lc); lc = new_lc; n = new_n;

    double a = 0;
    double b = 100;
    while(n!=1 && b-a>1){
        double epsilon = (a+b)/2;
        FILTER(lc, n, new_lc, new_n, i){
            if(frechet_dist(lc[i], c, epsilon)) ACCEPT;
        }
        if(new_n){
            n = new_n;
            free(lc);
            lc = new_lc;
        }else{
            a = epsilon;
            free(new_lc);
        }
    }

    if(n){
        draw_contour(img, lc[0], red);
    }


    image_save(img, argv[3]);
    image_free(img);
    free_contour_set(cs);
    free(level);

}
