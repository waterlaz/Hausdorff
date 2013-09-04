#include <stdio.h>
#include "contour.h"
#include "hausdorff.h"

int main(int argc, char** argv){
    if(argc<3){
        printf("Usage: %s <contourA> <contourB>\n", argv[0]);
        return 0;
    }
    contour_t a;
    contour_t b;
    read_contour(&a, argv[1]);
    read_contour(&b, argv[2]);

    int i;
    for(i=0; i<100; i++){
        double e = i/25.0;
        printf("%lf    %d\n", e, frechet_dist(a, b, e));
    }
}
