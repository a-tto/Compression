//
// Created by  AkyoTamabayashi on 2018/05/13.
//

#ifndef COMPRESSION_IMAGE_H
#define COMPRESSION_IMAGE_H

#include <stdio.h>
#include <stdlib.h>

#define TRUE 1;
#define FALSE 0;
#define PIXELMAX 255

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


ImageData *createImage(int width, int height, int depth);

void disposeImage(ImageData *img);
int getPixel(ImageData *img, int x, int y, Pixel *pixel);
int correctPixelValue(int value, int max);
int setPixel(ImageData *img, int x, int y, Pixel *pixel);

#endif //COMPRESSION_IMAGE_H
