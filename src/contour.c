#include <stdlib.h>
#include <stdio.h>

#include "contour.h"
#include "math.h"


contour_t alloc_contour(int n){
    contour_t res;
    res.n = n;
    res.points = (point_t*)malloc(sizeof(point_t)*n);
    return res;
}

void free_contour(contour_t contour){
    free(contour.points);
}

double point_scalar(point_t a, point_t b){
    return a.x*b.x + a.y*b.y;
}
    
point_t point_plus(point_t a, point_t b){
    point_t res;
    res.x = a.x + b.x;
    res.y = a.y + b.y;
    return res;
}

point_t point_minus(point_t a, point_t b){
    point_t res;
    res.x = a.x - b.x;
    res.y = a.y - b.y;
    return res;
}

double point_abs(point_t a){
    return sqrt(point_scalar(a, a));
}

point_t point_mul(double x, point_t a){
    point_t res;
    res.x = x*a.x;
    res.y = x*a.y;
    return res;
}

double point_distance(point_t a, point_t b){
    /* return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y)); */
    return point_abs(point_minus(b, a));
}

int read_contour(contour_t* contour, char* file_name){
    int n;
    FILE* f = fopen(file_name, "r");
    if(f==NULL){ 
        printf("Failed openning file %s \n", file_name); 
        return 1;
    }
    fscanf(f, "%d", &n);
    *contour = alloc_contour(n);
    int i;
    for(i=0; i<n; i++){
        double x, y;
        fscanf(f, "%lf %lf", &x, &y);
        contour->points[i].x = x;
        contour->points[i].y = y;
    }
    fclose(f);
    return 0;
}

int write_contour(contour_t* contour, char* file_name){
    FILE* f = fopen(file_name, "w");
    if(f==NULL){ 
        printf("Failed openning file %s for writing \n", file_name); 
        return 1;
    }
    fprintf(f, "%d\n", contour->n);
    int i;
    for(i=0; i<contour->n; i++){
        fprintf(f, "%lf %lf \n", contour->points[i].x, contour->points[i].y);
    }
    fclose(f);
    return 0;
}
