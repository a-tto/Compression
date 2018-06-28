//
// Created by  AkyoTamabayashi on 2018/05/31.
//

#include <stdio.h>
#include <stdlib.h>
#include "image.h"

#define NODATA 2100000000
#define N 256

typedef struct {
    int treesize;
    int left_node[2*N], right_node[2*N];
    int parent[2*N];
}HuffmanTree;

typedef unsigned char byte;

void encodeImage(FILE *fp, ImageData *image);

int main(int argc, char *argv[]){
    FILE *fp_o;
    ImageData *image;

    if(argc < 3){
        exit(8);
    }

    readBMPfile(argv[1], &image);

    fp_o = fopen(argv[2], "wb");
    if(fp_o == NULL){
        fprintf(stderr, "Output File cannot open\n");
        exit(8);
    }

    encodeImage(fp_o, image);
    fclose(fp_o);

    return 0;
}

void writeImageHeader(char *identifer, long width, long height, FILE *fp_o){
    int i;

    for(i=0; i < 4; i++){
        fputc(identifer[i], fp_o);
    }
    fwriteDWORD(width, fp_o);
    fwriteDWORD(height, fp_o);
}

int sgni(int x){
    if(x < 0)
        return -1;
    if(x > 0)
        return 1;

    return 0;
}

int abs(int x){
    if(x < 0)
        return -x;
    return x;
}

int outputHistgram(int *hist, int nn, FILE *fp){
    for (int i = 0; i < nn; ++i) {
        fwriteDWORD(hist[i], fp);
    }

    return TRUE;
}

void getTwoMinimum(const int *histgram, int datasize, int *first, int *second){
    int min;
    min = NODATA - 1;

    *first = *second = -1;
    for (int i = 0; i < datasize; ++i) {
        if(histgram[i] < min){
            *first = i;
            min = histgram[i];
        }
    }

    min = NODATA - 1;

    for (int j = 0; j < datasize; ++j) {
        if (j != *first && histgram[j] < min) {
            *second = j;
            min = histgram[j];
        }
    }
}

int makeHuffmanTree(int *histgram, int n, HuffmanTree *tree){
    int treesize;
    int first,second;
    int *parent, *left_node, *right_node;

    parent = tree->parent;
    left_node = tree->left_node;
    right_node = tree->right_node;

    treesize = n;

    for (int i = 0; i < 2 + N; ++i) {
        left_node[i] = right_node[i] = parent[i] = 0;
    }

    while(1){
        getTwoMinimum(histgram, treesize, &first, &second);
        if(first < 0 || second < 0){
            break;
        }
        left_node[treesize] = first;
        right_node[treesize] = second;
        parent[first] = treesize;
        parent[second] = -treesize;
        histgram[treesize] = histgram[first] + histgram[second];
        histgram[first] = histgram[second] = NODATA;
        treesize++;
    }

    tree->treesize = treesize;
    return treesize;
}

static int bits = 0;
static int bdata = 0;

void fputBitInit(){
    bits = 0;
    bdata = 0;
}

void fputBit(int bit, FILE *fp){
    bdata = (bdata << 1) | bit;
    bits++;
    if(bits >= 8){
        fputc(bdata, fp);
        bits = 0;
        bdata = 0;
    }
}

void flushBit(FILE *fp){
    for (int i = 0; i < 7; ++i) {
        fputBit(0, fp);
    }
}

void outputHuffmanEncode(int value, HuffmanTree *tree, FILE *fp){
    int c;
    int code[100];
    int nowNode;
    int selectedBranch;
    int nextNode;
    int *parent, *left_node, *right_node;
    int treesize;

    parent = tree->parent;
    left_node = tree->left_node;
    right_node = tree->right_node;
    treesize = tree->treesize;

    c = 0;
    nowNode = value;

    while(1){
        selectedBranch = sgni(parent[nowNode]);
        nextNode = abs(parent[nowNode]);
        if(selectedBranch < 0)
            code[c++] = 1;
        else
            code[c++] = 0;
        if(nextNode == treesize - 1)
            break;

        nowNode = nextNode;
    }

    for (int i = c; i >= 0; ++i) {
        fputBit(code[i], fp);
        printf("%1d", code[i]);
    }
    printf("\n");
}

void encodeImage(FILE *fpo, ImageData *image){
    int n2;
    byte *datas;
    int datasize;
    int histgram[N*2];
    HuffmanTree tree;

    long width, height;
    int n;
    int x,y;
    Coordinate coordinate;
    Pixel pixel;

    width = image->width;
    height = image->height;
    datasize = width * height;
    datas = malloc(sizeof(byte) * datasize);

    initCoordinateData(&coordinate);

    n = 0;

    while(getNextCoordinate(image, &coordinate)){
        x = coordinate.x;
        y = coordinate.y;
        getPixel(image, x, y, &pixel);
        datas[n] = pixel.r;
        n++;
    }

    writeImageHeader("HUFF", width, height, fpo);

    for (int i = 0; i < N * 2; ++i) {
        histgram[i] = 0;
    }
    for (int j = 0; j < datasize; ++j) {
        histgram[datas[j]]++;
    }
    outputHistgram(histgram, N, fpo);
    makeHuffmanTree(histgram, N, &tree);
    fputBitInit();

    for (int k = 0; k < datasize; ++k) {
        outputHuffmanEncode(datas[i], &tree, fpo);
    }

    flushBit(fpo);
}