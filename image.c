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

int  getPixel(ImageData *img, int x, int y, Pixel *pixel){
    int return_value = 1;
    int address;
    int depth, gray_value;
    BYTE *pixels;

    if(img == NULL)
        return -1;
    if(img->pixels == NULL)
        return -1;

    if(x < 0){
        x = 0;
        return_value = 0;
    }
    if(x >= img->width){
        x = img->width - 1;
        return_value = 0;
    }
    if(y < 0){
        y = 0;
        return_value = 0;
    }
    if(y >= img->height){
        y = img->height - 1;
        return_value = 0;
    }

    depth = img->depth;
    address = x + y*img->width;
    pixels = img->pixels;

    if(depth == 8){
        gray_value = pixels[address];
        pixel->r = gray_value;
        pixel->g = gray_value;
        pixel->b = gray_value;
    }else if(depth == 24){
        pixels += (address * 3);
        pixel->r = (*pixels);
        pixels++;
        pixel->g = (*pixels);
        pixels++;
        pixel->b = (*pixels);
    }else{
        return -1;
    }

    return return_value;
}