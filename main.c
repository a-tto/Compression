#include <stdio.h>
#include "image.h"


int main() {
    FILE *fp;

    ImageData *image;

    readBMPfile("/Users/akyo/compress/mono/LENNA.bmp",&image);

    writeBMPfile("/Users/akyo/compress/out2.bmp",image);

    return 0;
}