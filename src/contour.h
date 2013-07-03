#ifndef __CONTOUR_H_
#define __CONTOUR_H_ 

#include "image.h"

typedef struct _point_t {
    double x, y;
} point_t;

typedef struct _contour_t {
    int n;
    point_t* points;
} contour_t;

typedef struct _contour_set_t {
    contour_t* root;
    int n;  /* number of contours inside the current contours */
    contour_t* children;
} contour_t;

double point_distance(point_t a, point_t b);

contour_set_t* find_contours(image_t* img);


#endif
