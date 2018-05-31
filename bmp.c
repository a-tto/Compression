#include "image.h"



// BMPヘッダ部のデータ構造定義
typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPCOREHEADER {
    DWORD   bcSize;
    WORD    bcWidth;
    WORD    bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER{
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#define MAXCOLORS 256

// ファイルより２バイト整数を書き込む（リトルエンディアン）
int fwriteWORD(WORD val,FILE *fp)
{
    int i,c;

    c=val;
    for(i=0;i<2;i++) {
        fputc(c%256,fp);
        c/=256;
    }
    return TRUE;
}

// ファイルより４バイト整数を書き込む（リトルエンディアン）
int fwriteDWORD(DWORD val,FILE *fp)
{
    int i,c;

    c=val;
    for(i=0;i<4;i++) {
        fputc(c%256,fp);
        c/=256;
    }
    return TRUE;
}

// ファイルより２バイト整数を読み込む（リトルエンディアン）
int freadWORD(WORD *res,FILE *fp)
{
    int i,c;
    int val[2];

    for(i=0;i<2;i++) {
        c=fgetc(fp);
        if(c==EOF) return FALSE;
        val[i]=c;
    }
    *res=val[1]*256+val[0];
    return TRUE;
}

// ファイルより４バイト整数を読み込む（リトルエンディアン）
int  freadDWORD(DWORD *res,FILE *fp)
{
    int i,c;
    int val[4];
    DWORD tmp=0;

    for(i=0;i<4;i++) {
        c=fgetc(fp);
        if(c==EOF) return FALSE;
        val[i]=c;
    }
    tmp=0;
    for(i=3;i>=0;i--) {
        tmp*=256;
        tmp+=val[i];
    }
    *res=tmp;
    return TRUE;
}

// BMPの種類を判別
// 戻り値：FALSE　OS/2形式
//           TRUE   WIndows形式
static BOOL	IsWinDIB(BITMAPINFOHEADER* pBIH)
{
    if (((BITMAPCOREHEADER*)pBIH)->bcSize == sizeof(BITMAPCOREHEADER)) {
        return FALSE;
    }
    return TRUE;
}

// パレットのサイズを取得
// iBitCount １画素あたりのビット数
int countOfDIBColorEntries(int iBitCount)
{
    int	iColors;

    switch (iBitCount) {
        case 1:
            iColors	= 2;
            break;
        case 4:
            iColors	= 16;
            break;
        case 8:
            iColors	= 256;
            break;
        default:
            iColors	= 0;
            break;
    }

    return iColors;
}

// パディング要素を考慮して１列分のバイト数を求める
int getDIBxmax(int width,int depth)
{
    switch(depth) {
        case 32:
            return width*4;
        case 24:
            //return mx;
            return ((width*3)+3)/4*4;
            break;
        case 16:
            return (width+1)/2*2;
            break;
        case 8:
            return (width+3)/4*4;
            break;
        case 4:
            return (((width+1)/2)+3)/4*4;
            break;
        case 1:
            return (((width+7)/8)+3)/4*4;
    }
    return width;
}

// List2-7
// BMPデータをファイルより読み込む
int readBMPfile(char *filename,ImageData **image)
{
    int i,c;
    int errcode=0;
    BITMAPFILEHEADER BMPFile;
    BITMAPINFOHEADER BMPInfo;
    BITMAPCOREHEADER BMPCore;
    int	colors;
    int	colorTableSize;
    int	bitsSize;
    int	BISize;
    int x,y;
    int mx,my,depth;
    int pad;	//パディングするバイト数
    int mxbyte;	// １行を格納するのに必要なバイト数
    int isPM=FALSE;	// BMPの形式を記録するフラグ
    FILE *fp;

    WORD    HEAD_bfType;
    DWORD   HEAD_bfSize;
    WORD    HEAD_bfReserved1;
    WORD    HEAD_bfReserved2;
    DWORD   HEAD_bfOffBits;
    DWORD   INFO_bfSize;
    Pixel palet[MAXCOLORS];
    Pixel setcolor;

    if((fp=fopen(filename,"rb"))==NULL) {
        return -1;
    }

    // BMPファイルは必ず'BM(0x4d42)'で始まる．それ以外の場合はBMPではないので，中止する
    if(!freadWORD(&HEAD_bfType,fp)) {
        errcode=-2;
        goto $ABORT;
    }
    if (HEAD_bfType != 0x4d42) {
        errcode=-10;
        goto $ABORT;
    }
    // ヘッダ部のサイズ(Byte)
    if(!freadDWORD(&HEAD_bfSize,fp)) {
        errcode=-10;
        goto $ABORT;
    }
    // 予約用領域（未使用）
    if(!freadWORD(&HEAD_bfReserved1,fp)) {
        errcode=-10;
        goto $ABORT;
    }
    // 予約用領域（未使用）
    if(!freadWORD(&HEAD_bfReserved2,fp)) {
        errcode=-10;
        goto $ABORT;
    }
    // オフセット
    if(!freadDWORD(&HEAD_bfOffBits,fp)) {
        errcode=-10;
        goto $ABORT;
    }
    // ヘッダ部のサイズ
    if(!freadDWORD(&INFO_bfSize,fp)) {
        errcode=-10;
        goto $ABORT;
    }

    // ヘッダ部のサイズが規定外ならばエラーとする
    if (INFO_bfSize == 40/*sizeof(BITMAPINFOHEADER)*/ || INFO_bfSize == 12/*sizeof(BITMAPCOREHEADER)*/) {
        BMPInfo.biSize =	INFO_bfSize;
        // BITMAPCOREHEADER形式の場合
        if(INFO_bfSize == sizeof(BITMAPCOREHEADER)) {
            WORD tmp;
            isPM =	TRUE;
            // 画像の横幅
            if(!freadWORD(&tmp,fp)) {
                errcode=-10;
                goto $ABORT;
            }
            BMPInfo.biWidth=tmp;
            // 画像の縦幅
            if(!freadWORD(&tmp,fp)) {
                errcode=-10;
                goto $ABORT;
            }
            BMPInfo.biHeight=tmp;
            // 画像のプレーン数
            if(!freadWORD(&(BMPInfo.biPlanes),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // １画素あたりのビット数
            if(!freadWORD(&(BMPInfo.biBitCount),fp)) {
                errcode=-10;
                goto $ABORT;
            }
        }
        else {		// BITMAPINFOHEADER形式の場合
            // 画像の横幅
            if(!freadDWORD(&(BMPInfo.biWidth),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // 画像の縦幅
            if(!freadDWORD(&(BMPInfo.biHeight),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // 画像のプレーン数
            if(!freadWORD(&(BMPInfo.biPlanes),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // １画素あたりのビット数
            if(!freadWORD(&(BMPInfo.biBitCount),fp)) {
                errcode=-10;
                goto $ABORT;
            }
        }

        // BITMAPINFOHEADERの場合のみ存在する情報を読み込む
        if(!isPM) {
            // 圧縮形式
            if(!freadDWORD(&(BMPInfo.biCompression),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // 画像データ部のサイズ
            if(!freadDWORD(&(BMPInfo.biSizeImage),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // X方向の解像度
            if(!freadDWORD(&(BMPInfo.biXPelsPerMeter),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // Y方向の解像度
            if(!freadDWORD(&(BMPInfo.biYPelsPerMeter),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // 格納されているパレットの色数
            if(!freadDWORD(&(BMPInfo.biClrUsed),fp)) {
                errcode=-10;
                goto $ABORT;
            }
            // 重要なパレットのインデックス
            if(!freadDWORD(&(BMPInfo.biClrImportant),fp)) {
                errcode=-10;
                goto $ABORT;
            }
        }
    }
    else {
        errcode=-10;
        goto $ABORT;
    }
    mx=BMPInfo.biWidth;
    my=BMPInfo.biHeight;
    depth=BMPInfo.biBitCount;
    // 256色，フルカラー以外サポート外
    if(depth!=8 && depth!=24) {
        errcode=-3;
        goto $ABORT;
    }
    // 非圧縮形式以外はサポート外
    if(BMPInfo.biCompression!=BI_RGB) {
        errcode=-20;
        goto $ABORT;
    }
    // ヘッダ部にパレットサイズの情報がない場合は１画素あたりのビット数から求める
    if(BMPInfo.biClrUsed==0) {
        colors	= countOfDIBColorEntries(BMPInfo.biBitCount);
    }
    else {
        colors	= BMPInfo.biClrUsed;
    }

    // パレット情報の読み込み
    // BMPの種類によってフォーマットが異なるので処理をわける
    if (!isPM)	{
        for(i=0;i<colors;i++) {
            // Blue成分
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].b=c;
            // Green成分
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].g=c;
            // Red成分
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].r=c;
            // あまり
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
        }
    } else {
        for(i=0;i<colors;i++) {
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].b=c;
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].g=c;
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-10;
                goto $ABORT;
            }
            palet[i].r=c;
        }
    }
    // フルカラーで画像データを作成
    *image=createImage(mx,my,24);
    mxbyte=getDIBxmax(mx,depth);
    pad=mxbyte-mx*depth/8;
    // 画像データ読み込み
    for(y=my-1;y>=0;y--) {
        for(x=0;x<mx;x++) {
            if(depth==8) {	// 256色形式の場合はパレットからRGB値を求める
                c=fgetc(fp);
                if(c==EOF) {
                    errcode=-20;
                    goto $ABORT;
                }
                setcolor.r=palet[c].r;
                setcolor.g=palet[c].g;
                setcolor.b=palet[c].b;
            }
            else if(depth==24) {
                c=fgetc(fp);
                if(c==EOF) {
                    errcode=-20;
                    goto $ABORT;
                }
                setcolor.b=c;
                c=fgetc(fp);
                if(c==EOF) {
                    errcode=-20;
                    goto $ABORT;
                }
                setcolor.g=c;
                c=fgetc(fp);
                if(c==EOF) {
                    errcode=-20;
                    goto $ABORT;
                }
                setcolor.r=c;
            }
            setPixel(*image,x,y,&setcolor);
        }
        // Padding部の読み飛ばし
        for(i=0;i<pad;i++) {
            c=fgetc(fp);
            if(c==EOF) {
                errcode=-20;
                goto $ABORT;
            }
        }
    }
    $ABORT:	// エラー時の飛び先
    fclose(fp);
    return errcode;
}

// List2-8
// 画像データをBMP形式(Windows形式)でファイルに書き出す
// （フルカラーの画像データのみサポート）
int writeBMPfile(char *filename,ImageData *image)
{
    FILE *fp;
    BITMAPFILEHEADER bfn;
    int width,height;
    int mxbyte;	// １行を格納するのに必要なバイト数
    //int x
    int pad;
    int depth;	// １画素あたりのビット数
    int byte_per_pixel;	//１要素あたりのバイト数
    int palet_size;	// パレットサイズ（未実装）
    int x,y,i;
    int	iBytes;
    //unsigned int wsize;
    Pixel pixel;

    width=image->width;
    height=image->height;
    depth=image->depth;

    // フルカラー以外サポート外
    if(depth!=24) {
        //errcode=-3;
        goto $abort1;
    }
    // フルカラー以外のことを若干考慮しているが未実装．フルカラーのみなら、本来-- end -- の部分まで不要
    if(depth==24) {
        byte_per_pixel=1;
    }
    else {
        byte_per_pixel=depth/8;
    }
    if(depth>=24) {
        palet_size=0;
    }
    else {
        palet_size=256;
    }
    // -- end --
    // パディングを考慮した１列分に必要なバイト数
    mxbyte=getDIBxmax(width,depth);
    // ヘッダ部の設定（一部のみ）
    bfn.bfType=0x4d42;	//'BM'
    bfn.bfSize=14+40+/*sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER) +*/
               palet_size*4/*sizeof(RGBQUAD)*/ +
               mxbyte*height*byte_per_pixel;
    bfn.bfReserved1=0;
    bfn.bfReserved2=0;
    bfn.bfOffBits=14+40/*sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER)*/ +
                  palet_size*4/*sizeof(RGBQUAD)*/;

    if((fp=fopen(filename,"wb"))==NULL) {
        goto $abort1;
    }
    // ヘッダ部の書き出し
    fwriteWORD(bfn.bfType,fp);
    fwriteDWORD(bfn.bfSize,fp);
    fwriteWORD(bfn.bfReserved1,fp);
    fwriteWORD(bfn.bfReserved2,fp);
    fwriteDWORD(bfn.bfOffBits,fp);
    fwriteDWORD(40/*sizeof(BITMAPINFOHEADER)*/,fp);	//biSize
    fwriteDWORD(width,fp);		// biWidth
    fwriteDWORD(height,fp);		// biHeight
    fwriteWORD(1,fp);		// biPlanes
    fwriteWORD(depth,fp);	// biBitCount
    fwriteDWORD(BI_RGB,fp);	// biCompression
    fwriteDWORD(0,fp);	// biSizeImage
    fwriteDWORD(300,fp);	// biXPelsPerMeter
    fwriteDWORD(300,fp);	// biYPelsPerMeter
    fwriteDWORD(0,fp);		// biClrUsed
    fwriteDWORD(0,fp);		// biClrImportant

    // 必要なパディングのサイズ
    pad=mxbyte-width*depth/8;
    // 画像データの書き出し
    for(y=height-1;y>=0;y--) {
        for(x=0;x<width;x++) {
            getPixel(image,x,y,&pixel);
            fputc(pixel.b,fp);
            fputc(pixel.g,fp);
            fputc(pixel.r,fp);
        }
        // Padding部の出力
        for(i=0;i<pad;i++) {
            fputc(0,fp);
        }
    }
    return 0;
    $abort1:
    return 1;
    $abort2:
    fclose(fp);
    return 1;
}
