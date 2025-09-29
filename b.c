#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#undef main

#define IN_W 512  // 影像寬度
#define IN_H 512  // 影像高度

#pragma pack(1)  // 結構體 1-byte 對齊，確保 BMP 讀取正確
typedef struct {
    unsigned short bfType;       // BMP 檔案類型
    unsigned int bfSize;         // 檔案大小
    unsigned short bfReserved1;  // 保留
    unsigned short bfReserved2;  // 保留
    unsigned int bfOffBits;      // 從檔頭到像素資料的位移
} BITMAPFILEHEADER;

typedef struct {
    unsigned int biSize;          // BITMAPINFOHEADER大小
    int biWidth;                  // 影像寬度
    int biHeight;                 // 影像高度
    unsigned short biPlanes;      // 色彩平面數
    unsigned short biBitCount;    // 每像素位元數
    unsigned int biCompression;   // 壓縮方式
    unsigned int biSizeImage;     // 影像資料大小
    int biXPelsPerMeter;          // 水平解析度
    int biYPelsPerMeter;          // 垂直解析度
    unsigned int biClrUsed;       // 調色盤顏色數量
    unsigned int biClrImportant;  // 重要顏色數量
} BITMAPINFOHEADER;
#pragma pack()

// 讀取 8-bit 灰階 BMP 影像
int readBMPGray8(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror(filename); return 0; }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    fread(&fileHeader, sizeof(fileHeader), 1, fp);
    fread(&infoHeader, sizeof(infoHeader), 1, fp);

    // 檢查影像尺寸與位元深度是否符合要求
    if (infoHeader.biWidth != IN_W || infoHeader.biHeight != IN_H || infoHeader.biBitCount != 8) {
        printf("BMP size or format not supported (need 512x512 8bit): %s\n", filename);
        fclose(fp); return 0;
    }

    fseek(fp, fileHeader.bfOffBits, SEEK_SET); // 移動到像素資料位置
    for (int y = IN_H - 1; y >= 0; y--)  // BMP 從下往上存
        fread(image[y], 1, IN_W, fp);

    fclose(fp);
    return 1;
}

// 讀取 RAW 影像 (純像素資料)
int readRAW(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror(filename); return 0; }
    size_t readBytes = fread(image, sizeof(unsigned char), IN_W*IN_H, fp);
    fclose(fp);
    return (readBytes == IN_W*IN_H);  // 檢查是否讀取完整
}

// 對影像做對數變換 (log transform)
void logTransform(unsigned char in[IN_H][IN_W], unsigned char out[IN_H][IN_W]) {
    double c = 255.0 / log(1 + 255.0);  // 對數變換常數
    for (int y=0;y<IN_H;y++)
        for (int x=0;x<IN_W;x++)
            out[y][x] = (unsigned char)(c * log(1 + in[y][x]));
}

// 對影像做 gamma 變換
void gammaTransform(unsigned char in[IN_H][IN_W], unsigned char out[IN_H][IN_W], double gamma) {
    double c = 255.0;  // 常數
    for (int y=0;y<IN_H;y++)
        for (int x=0;x<IN_W;x++)
            out[y][x] = (unsigned char)(c * pow(in[y][x]/255.0, gamma));
}

// 對影像做負片變換 (negative transform)
void negativeTransform(unsigned char in[IN_H][IN_W], unsigned char out[IN_H][IN_W]) {
    for (int y=0;y<IN_H;y++)
        for (int x=0;x<IN_W;x++)
            out[y][x] = 255 - in[y][x];
}

// 顯示灰階影像 (手動關閉視窗才會跳下一張)
void showImage(unsigned char* img, int w, int h, const char* title) {
    SDL_Window* win = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w,h,0);
    SDL_Renderer* ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, w,h);

    // 灰階轉 RGB
    unsigned char* rgb = (unsigned char*)malloc(w*h*3);
    for (int i=0;i<w*h;i++){
        rgb[i*3+0] = img[i];  // B
        rgb[i*3+1] = img[i];  // G
        rgb[i*3+2] = img[i];  // R
    }
    SDL_UpdateTexture(tex, NULL, rgb, w*3);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);
    free(rgb);

    // 等待視窗關閉
    SDL_Event e;
    int quit=0;
    while (!quit) {
        if (SDL_WaitEvent(&e)) {
            if (e.type==SDL_QUIT) quit=1;  // 只在視窗被關閉時才返回
        }
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
}

int main() {
    const char* files[6] = {
        "images/baboon.bmp","images/boat.bmp","images/F16.bmp",
        "images/goldhill.raw","images/lena.raw","images/peppers.raw"
    };

    unsigned char input[IN_H][IN_W], out[IN_H][IN_W];

    SDL_Init(SDL_INIT_VIDEO);  // 初始化 SDL

    for (int i=0;i<6;i++){
        int ok=0;
        // 根據檔案副檔名選擇讀取函式
        if(strstr(files[i],".bmp")) ok=readBMPGray8(files[i],input);
        else ok=readRAW(files[i],input);
        if(!ok){printf("Skip %s\n",files[i]);continue;}

        // 對影像做對數變換並顯示
        logTransform(input,out);
        showImage((unsigned char*)out,IN_W,IN_H,"Log transform");

        // 對影像做 gamma 變換並顯示
        gammaTransform(input,out,0.5);
        showImage((unsigned char*)out,IN_W,IN_H,"Gamma transform");

        // 對影像做負片變換並顯示
        negativeTransform(input,out);
        showImage((unsigned char*)out,IN_W,IN_H,"Negative transform");
    }

    SDL_Quit();  // 結束 SDL
    return 0;
}
