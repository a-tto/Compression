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
typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned short      WORD;
typedef unsigned long       LONG;

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L


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

typedef struct {
    int x,y;
    int buffer1,buffer2,buffer3;
}Coordinate;

ImageData *createImage(int width, int height, int depth);

void disposeImage(ImageData *img);
int readBMPfile(char *fname,ImageData **img);
int writeBMPfile(char *fname,ImageData *img);
int getPixel(ImageData *img, int x, int y, Pixel *pixel);
int correctPixelValue(int value, int max);
int setPixel(ImageData *img, int x, int y, Pixel *pixel);
int fwriteDWORD(DWORD val, FILE *fp);
int initCoordinateData(Coordinate *coordinate);
int getNextCoordinate(ImageData *image, Coordinate *coodinate);
#endif //COMPRESSION_IMAGE_H
