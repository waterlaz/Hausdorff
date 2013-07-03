#ifndef __HAUSDORFF_H_
#define __HAUSDORFF_H_ 

#include "array2d.h"
#include "contour.h"



double hausdorff_discrete_dist(contour_t a, contour_t b){
    int n = a.n + 1;
    int m = 2*b.n + 1;
    double f**;
    ALLOC_2DARRAY(f, n, m, double);
    int i. j;
    for(i=0; i<n; i++)
    for(j=0; j<m; j++)
        f[i][j] = point_distance(a.points[i%a.n], b.points[j%b.n]);
    
    FREE_2DARRAY(f, n, m);
}

#endif
