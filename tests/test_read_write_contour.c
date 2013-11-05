#include <stdio.h>
#include "contour.h"

int main(){
    contour_t* c = read_contour("input_contour");
    write_contour(c, "output_contour");
}
