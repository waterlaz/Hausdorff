#include "contour.h"
#include "math.h"

double point_distance(point_t a, point_t b){
    return sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
}
