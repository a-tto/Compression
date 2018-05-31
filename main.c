#include <stdio.h>
#include "image.h"


int main() {
    ImageData *image;
    Coordinate *coordinate;
    int x,y;
    int width,height;

    image = createImage(256,256,24);

    width = image->width;
    height = image->height;

    for(y=0; y < height; y++){
        for(x=0; x < width; x++){
            //画像処理
        }
    }

    disposeImage(image);

    return 0;
}