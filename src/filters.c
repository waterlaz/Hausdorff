#include <stdlib.h>

#include "filters.h"
#include "image.h"


#define elem_type int
#define ELEM_SWAP(a,b) { register elem_type t=(a);(a)=(b);(b)=t; }

elem_type quick_select(elem_type arr[], int n) 
{
    int low, high ;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) /* One element only */
            return arr[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

    /* Find median of low, middle and high items; swap into position low */
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

    /* Swap low item (now in position middle) into position (low+1) */
    ELEM_SWAP(arr[middle], arr[low+1]) ;

    /* Nibble from each end towards middle, swapping items when stuck */
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (arr[low] > arr[ll]) ;
        do hh--; while (arr[hh]  > arr[low]) ;

        if (hh < ll)
        break;

        ELEM_SWAP(arr[ll], arr[hh]) ;
    }

    /* Swap middle item (in position low) back into correct position */
    ELEM_SWAP(arr[low], arr[hh]) ;

    /* Re-set active partition */
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }
}


pixel_t pixel_median(image_t* img, int x, int y, int d, int* r,  int* g, int* b){
	int dx, dy; 
	int i=0;
	for(dx=x-d; dx<=x+d; dx++)
		for(dy=y-d; dy<=y+d; dy++){
			if(pixel_diff(&img->img[dx][dy], &img->img[x][y])<2700){
				r[i]=img->img[dx][dy].r;
				g[i]=img->img[dx][dy].g;
				b[i]=img->img[dx][dy].b;
				i++;
			}
		}
	pixel_t res;
	res.r = quick_select(r, i);
	res.g = quick_select(g, i);
	res.b = quick_select(b, i);
	return res;
}
	

image_t* filter_median(image_t* img, int d){
	image_t* res = image_copy(img);
	int l = (2*d+1)*(2*d+1);
	int* r = (int*)malloc(sizeof(int)*l);
	int* g = (int*)malloc(sizeof(int)*l);
	int* b = (int*)malloc(sizeof(int)*l);
	int x, y;
	for(x=d; x<img->w-d; x++){
		printf("%d/%d\n", x, img->w);
		for(y=d; y<img->h-d; y++){
			res->img[x][y] = pixel_median(img, x, y, d, r, g, b);
		}
	}
	return res;
	free(r);
	free(g);
	free(b);
}
