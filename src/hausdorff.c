#include <math.h>
#include <stdlib.h>

#include "hausdorff.h"
#include "array2d.h"
#include "contour.h"

double sqr(double x){
    return x*x;
}

void interval(point_t x, point_t y1, point_t y2, double* a, double* b, double eps){
    point_t dy;
    dy = point_minus(y2, y1);
    double alpha = point_scalar(dy, 
                                point_minus(x, y1))
                      /
                   (sqr(point_abs(dy)));
    point_t theta = point_plus(point_mul(alpha, dy), y1);
    double sqr_delta = eps*eps - sqr(point_distance(theta, x));
    if(sqr_delta<0){
        *a = 1;
        *b = -1;
        return;
    }
    double delta = sqrt(sqr_delta);
    *a = alpha*point_abs(dy)-delta;
    *b = alpha*point_abs(dy)+delta;
}

void push(double a1, double b1, double* a2, double* b2){
    if (a1>*a2) *a2 = a1;
    if (a1>b1){
        *a2 = 1;
        *b2 = -1;
    }
}

int frechet_dist(contour_t a, contour_t b, double eps){   
    int n = a.n + 1;
    int m = 2*b.n + 1;
    
    double** ha;
    double** hb;
    double** va;
    double** vb;
    
    ALLOC_2DARRAY(ha, n, m, double);
    ALLOC_2DARRAY(hb, n, m, double);
    ALLOC_2DARRAY(va, n, m, double);
    ALLOC_2DARRAY(vb, n, m, double);
    
    int i, j;
    for(i=0; i<n; i++)
    for(j=0; j<m; j++){
        interval(a.points[i%a.n], b.points[j%b.n], b.points[(j+1)%b.n], &va[i][j], &vb[i][j], eps);
        interval(b.points[j%b.n], a.points[i%a.n], a.points[(i+1)%a.n], &ha[i][j], &hb[i][j], eps);       
    }
    
    for(i=0; i<n; i++){
        ha[i][0] = 1;
        hb[i][0] = -1;
        ha[i][m-1] = 1;
        hb[i][m-1] = -1;
    }
    
    for(j=0; j<m; j++){
        if(j<b.n){ 
            va[n-1][j] = 1;
            vb[n-1][j] = -1;
        }else{
            va[0][j] = 1;
            vb[0][j] = -1;
        }
        
    }
    /*
    for(i=0; i<n; i++){
    for(j=0; j<m; j++){
        printf("%lf %lf \n", va[i][j], vb[i][j]);
        printf("%lf %lf \n \n", ha[i][j], hb[i][j]);
        
    }
        printf("##########\n");
    }
    */
    
    int z = 300;
    while(z--){
        for(i=0; i<n-1; i++)
        for(j=0; j<m-1; j++){
            if(va[i][j] > vb[i][j]) push(ha[i][j], hb[i][j], &ha[i][j+1], &hb[i][j+1]);
            if(ha[i][j] > hb[i][j]) push(va[i][j], vb[i][j], &va[i+1][j], &vb[i+1][j]);
        }
        for(j=0; j<b.n;j++){
            va[0][j] = va[n-1][j+b.n];
        }
    
    }
    
    for(j=b.n; j<2*b.n; j++){
        if(va[a.n][j]<vb[a.n][j]) return 1;
    }
    
    return 0;
    
    FREE_2DARRAY(ha, n, m);
    FREE_2DARRAY(hb, n, m);
    FREE_2DARRAY(va, n, m);
    FREE_2DARRAY(vb, n, m);
}
