#include <IL/il.h> 
#include <IL/ilu.h> 
#include <stdlib.h>

#include "image.h"
#include "array2d.h"

image_t* image_create(int w, int h){
    image_t* image = (image_t*)malloc(sizeof(image_t));
    image->w = w;
    image->h = h;
    ALLOC_2DARRAY(image->img, w, h, pixel_t);
    return image;
}

image_t* image_copy(image_t* img){
	image_t* res = image_create(img->w, img->h);
	int x, y;
	for(x=0; x<img->w; x++)
	for(y=0; y<img->h; y++)
		res->img[x][y] = img->img[x][y];
	return res;
}


void image_free(image_t* img){
    FREE_2DARRAY(img->img, img->w, img->h);
    free(img);
}

pixel_t pixel_negative(pixel_t pixel){
    pixel_t res;
    res.r = MAX_COLOR - pixel.r;
    res.g = MAX_COLOR - pixel.g;
    res.b = MAX_COLOR - pixel.b;
    return res;
}

image_t* image_negative(image_t* img){
    image_t* res = image_copy(img);
    int i, j;
    for(i=0; i<img->w; i++)
        for(j=0; j<img->h; j++){
            res->img[i][j] = pixel_negative(img->img[i][j]);
        }
    return res;
}

void image_draw_line(image_t* img, int x1, int y1, int x2, int y2, pixel_t color){
	int dx = x2>x1 ? 1 : -1;
	int dy = y2>y1 ? 1 : -1;
	int x = x1; 
	int y = y1;
	while(x!=x2 || y!=y2){
		if(x>=0 && y>=0 && x<img->w && y<img->h)img->img[x][y]=color;
		if(abs((x+dx-x1)*(y2-y1)-(y-y1)*(x2-x1))<abs((x-x1)*(y2-y1)-(y+dy-y1)*(x2-x1))){
			x+=dx;
		} else y+=dy;
	}
}

void init_image_lib(){
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);
	iluInit();
}

image_t* image_load(char *s){
	ILuint texid;
	ilGenImages(1, &texid);
	ilBindImage(texid);
	if(!ilLoadImage(s)){
		printf("Failed loading image %s\n", s);
		goto fail;
	}
	int w = ilGetInteger(IL_IMAGE_WIDTH);
	int h = ilGetInteger(IL_IMAGE_HEIGHT);
	char* pixels = (char*)malloc(sizeof(char)*w*h*3);
	unsigned char* p = pixels;
	ilCopyPixels(0, 0, 0, w, h, 1, IL_RGB, IL_UNSIGNED_BYTE, pixels);
	image_t* res = image_create(w, h);
	int x, y;
	for(y=0; y<h; y++)
		for(x=0; x<w; x++){
			res->img[x][y].r = *p; p++;
			res->img[x][y].g = *p; p++;
			res->img[x][y].b = *p; p++;
		}
	free(pixels);
	ilDeleteImages(1, &texid);
	return res;
	fail: ;
	ILenum Error;
	while ((Error = ilGetError()) != IL_NO_ERROR) {
		printf("%d: %s\n", Error, iluErrorString(Error));
	}
	ilDeleteImages(1, &texid);
	return NULL;
}

int image_save(image_t* img, char *s){
	ILuint texid;
	ilGenImages(1, &texid);
	ilBindImage(texid);
	
	char* pixels = (char*)malloc(sizeof(char)*img->w*img->h*3);
	char* p = pixels;
	int x, y;
	for(y=0; y<img->h;y++){
		for(x=0; x<img->w;x++){
			*p = img->img[x][y].r; p++;
			*p = img->img[x][y].g; p++;
			*p = img->img[x][y].b; p++;
		}
	}
	
	if(!ilTexImage(img->w, img->h, 1, 3, IL_RGB, IL_UNSIGNED_BYTE, pixels)){
		printf("Failed converting image\n");
		goto fail;
	}
	
	iluFlipImage();
	
	if(!ilSaveImage(s)){
		printf("Failed saving image to %s\n", s);
		goto fail;
	}
	ilDeleteImages(1, &texid);
	return 0;
	fail: ;
	ILenum Error;
	while ((Error = ilGetError()) != IL_NO_ERROR) {
		printf("%d: %s\n", Error, iluErrorString(Error));
	}
    free(pixels);
	ilDeleteImages(1, &texid);
	return 1;
}
