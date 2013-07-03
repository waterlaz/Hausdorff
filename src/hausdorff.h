#ifndef __HAUSDORFF_H_
#define __HAUSDORFF_H_ 

#include "array2d.h"
#include "contour.h"


void field_discrete_trans(double** f, int n, int m){
    int i, j;
    for(j=0; j<m/2;j++)
    for(i=0; i<n; i++){
        f[i][j] = f[n-i-1][m-j-1];
    }
    for(i=0; i<n/2; i++)
        f[i][m/2] = f[n-i-1][m/2];
}

int atomic_descrete_op(double **f, int x, int y){
    min3()
}

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
