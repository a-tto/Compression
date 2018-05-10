//
// Created by  AkyoTamabayashi on 2018/05/10.
//
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;

typedef struct {
    int width,height;
    int depth;
    void *pixels;
} ImageData;

typedef struct {
    int r;
    int g;
    int b;
}Pixel;

ImageData *createImage(int width, int height, int depth){
    ImageData *new_image;
    int byte_per_pixel;

    if(width < 0 || height < 0)
        return NULL;
    if(depth != 8 && depth != 24)
        return NULL;

    new_image = malloc(sizeof(ImageData));
    if(new_image == NULL)
        return NULL;

    byte_per_pixel = depth/8;

    new_image->pixels = malloc(sizeof(BYTE)*byte_per_pixel*width*height);
    if(new_image->pixels == NULL){
        free(new_image);
        return NULL;
    }

    new_image->width = width;
    new_image->height = height;
    new_image->depth = depth;

    return new_image;
}

void disposeImage(ImageData *img){
    if(img->pixels != NULL)
        free(img->pixels);

    free(img);
}

