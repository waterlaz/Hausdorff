#include <IL/il.h> 

#include "image.h"
#include "array2d.h"

image_t* image_create(int w, int h){
    image_t* image = (image_t*)malloc(sizeof(image_t));
    image->w = w;
    image->h = h;
    ALLOC_2DARRAY(image->img, w, h, pixel_t);
    return image;
}

void init_image_lib(){
	ilInit();
}

image_t* image_load(char *s){
	ILuint texid;
	ilGenImages(1, &texid);
	ilBindImage(texid);
	if(!ilLoadImage(s)){
		printf("Failed loading image %s\n", s);
		goto fail;
	}
	if(!ilConvertImage(IL_RGB, IL_BYTE)){
		printf("Failed converting image to RGB\n");
		goto fail;
	}
	char* pixels = (char*)ilGetData();
	int w = ilGetInteger(IL_IMAGE_WIDTH);
	int h = ilGetInteger(IL_IMAGE_HEIGHT);
	image_t* res = image_create(w, h);
	int x, y;
	for(y=0; y<h; y++)
		for(x=0; x<w; x++){
			res->img[x][y].r = *pixels; pixels++;
			res->img[x][y].g = *pixels; pixels++;
			res->img[x][y].b = *pixels; pixels++;
		}
	ilDeleteImages(1, &texid);
	return res;
	fail: ;
	ilDeleteImages(1, &texid);
	return NULL;
}

void image_save(image_t* img, char *s){
	ILuint texid;
	ilGenImages(1, &texid);
	if(!ilSaveImage(s)){
		printf("Failed saving image to %s\n", s);
	}
}
