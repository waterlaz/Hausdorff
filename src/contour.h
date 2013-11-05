#ifndef __CONTOUR_H_
#define __CONTOUR_H_ 

#include "image.h"

typedef struct _point_t {
    double x, y;
} point_t;


typedef struct _contour_bounding_box_t{
    double x1, y1, x2, y2;
} contour_bounding_box_t;

typedef struct _contour_meta_t {
    double* area;
    double* perimeter;
    contour_bounding_box_t* bounding_box;
} contour_meta_t;

typedef struct _contour_t {
    int n;
    point_t* points;
    contour_meta_t meta;
} contour_t;

#define FOR_CONTOUR_POINTS(c, p) for(point_t* p=c->points; p<c->points+c->n; p++)

typedef struct _contour_set_t {
    contour_t* root;
    int n;  /* number of contours inside the current contours */
    struct _contour_set_t* children;
} contour_set_t;


/* allocation and deallocation of the memory for contours */
contour_t* alloc_contour(int n);

void free_contour(contour_t* contour);

/* check if bounding box a is inside bounding box b */
int is_box_inside(contour_bounding_box_t* a, contour_bounding_box_t* b);

/* Scalar product of two points */
double point_scalar(point_t a, point_t b);

/* Vector sum of two points */
point_t point_plus(point_t a, point_t b);

/* Vector difference of two points */
point_t point_minus(point_t a, point_t b);

/* Product of a scalar on a point */
point_t point_mul(double x, point_t a);

/* The length of the point as a vector */
double point_abs(point_t a);

/* Distance between two points */
double point_distance(point_t a, point_t b);

/* Find the contours on the image */
contour_set_t* find_contours(image_t* img, int n_levels, int* level);

/* Reads the contour from file file_name */
/* The format has the following form: 
 * <n - number of points>
 * x1 y1
 * x2 y2
 * ...
 * xn yn */
contour_t* read_contour(char* file_name);

/* Write the contour to file file_name */
int write_contour(contour_t* contour, char* file_name);

#endif
