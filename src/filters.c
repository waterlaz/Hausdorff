#include <stdlib.h>

#include "filters.h"
#include "image.h"

int compare_ints(const int *a, const int *b){
  int temp = *a - *b;
  if (temp > 0)
    return 1;
  else if (temp < 0)
    return -1;
  else
    return 0;
}

pixel_t pixel_median(image_t* img, int x, int y, int d){
	int dx, dy;
	int l = (2*d+1)*(2*d+1);
	int* r = (int*)malloc(sizeof(int)*l);
	int* g = (int*)malloc(sizeof(int)*l);
	int* b = (int*)malloc(sizeof(int)*l);
	int i=0;
	for(dx=x-d; dx<=x+d; dx++)
		for(dy=y-d; dy<=y+d; dy++){
			r[i]=img->img[dx][dy].r;
			g[i]=img->img[dx][dy].g;
			b[i]=img->img[dx][dy].b;
			i++;
		}
	qsort(r, l, sizeof(int), compare_ints);
	qsort(g, l, sizeof(int), compare_ints);
	qsort(b, l, sizeof(int), compare_ints);
	pixel_t res;
	res.r = r[l/2];
	res.g = g[l/2];
	res.b = b[l/2];
	return res;
}
	

image_t* filter_median(image_t* img, int d){
	image_t* res = image_copy(img);
	int x, y;
	for(x=d; x<img->w-d; x++){
		printf("%d/%d\n", x, img->w);
		for(y=d; y<img->h-d; y++){
			res->img[x][y] = pixel_median(img, x, y, d);
		}
	}
	return res;
}
