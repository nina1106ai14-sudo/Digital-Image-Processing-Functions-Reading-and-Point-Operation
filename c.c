#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#undef main

#define IN_W 512  // 影像寬度
#define IN_H 512  // 影像高度

// ---------------------- 讀取 RAW 影像 (512x512) ----------------------
int readRAW(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror(filename); return 0; }

    // 讀取影像資料
    size_t readBytes = fread(image, sizeof(unsigned char), IN_W * IN_H, fp);
    fclose(fp);

    // 檢查是否讀取完整
    if (readBytes != IN_W * IN_H) {
        printf("RAW read failed (read %zu bytes, expected %d)\n", readBytes, IN_W * IN_H);
        return 0;
    }
    return 1;
}

// ---------------------- BMP 結構體 ----------------------
#pragma pack(1) // 1-byte 對齊，確保 BMP 讀取正確
typedef struct {
    uint16_t bfType;       // BMP 檔案類型
    uint32_t bfSize;       // 檔案大小
    uint16_t bfReserved1;  // 保留
    uint16_t bfReserved2;  // 保留
    uint32_t bfOffBits;    // 從檔頭到像素資料的位移
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;         // BITMAPINFOHEADER 大小
    int32_t  biWidth;        // 影像寬度
    int32_t  biHeight;       // 影像高度
    uint16_t biPlanes;       // 色彩平面數
    uint16_t biBitCount;     // 每像素位元數
    uint32_t biCompression;  // 壓縮方式
    uint32_t biSizeImage;    // 影像資料大小
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BITMAPINFOHEADER;
#pragma pack()

// ---------------------- 讀取 8-bit 灰階 BMP ----------------------
int readBMPGray(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { perror(filename); return 0; }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    fread(&fileHeader, sizeof(fileHeader), 1, fp);
    fread(&infoHeader, sizeof(infoHeader), 1, fp);

    // 確認尺寸與位元深度
    if (infoHeader.biWidth != IN_W || abs(infoHeader.biHeight) != IN_H || infoHeader.biBitCount != 8) {
        printf("BMP not 512x512 8bit: %s\n", filename);
        fclose(fp); return 0;
    }

    // 跳到像素資料位置
    fseek(fp, fileHeader.bfOffBits, SEEK_SET);

    // BMP 從下往上存，需倒序讀取
    for (int y = IN_H - 1; y >= 0; y--) {
        fread(image[y], sizeof(unsigned char), IN_W, fp);
    }

    fclose(fp);
    return 1;
}

// ---------------------- 最近鄰插值 ----------------------
void resizeNearest(unsigned char* src, int sw, int sh,
                unsigned char* dst, int dw, int dh) {
    for (int y = 0; y < dh; y++) {
        int sy = (int)((float)y / dh * sh); // 計算對應原圖 y
        if (sy >= sh) sy = sh - 1;
        for (int x = 0; x < dw; x++) {
            int sx = (int)((float)x / dw * sw); // 計算對應原圖 x
            if (sx >= sw) sx = sw - 1;
            dst[y*dw + x] = src[sy*sw + sx]; // 複製像素
        }
    }
}

// ---------------------- 雙線性插值 ----------------------
void resizeBilinear(unsigned char* src, int sw, int sh,
                    unsigned char* dst, int dw, int dh) {
    for (int y = 0; y < dh; y++) {
        float gy = ((float)y / (dh - 1)) * (sh - 1);
        int y0 = (int)floorf(gy);
        int y1 = y0 + 1; if (y1 >= sh) y1 = sh - 1;
        float dy = gy - y0;
        for (int x = 0; x < dw; x++) {
            float gx = ((float)x / (dw - 1)) * (sw - 1);
            int x0 = (int)floorf(gx);
            int x1 = x0 + 1; if (x1 >= sw) x1 = sw - 1;
            float dx = gx - x0;

            // 先做水平方向內插
            float v0 = (1 - dx)*src[y0*sw + x0] + dx*src[y0*sw + x1];
            float v1 = (1 - dx)*src[y1*sw + x0] + dx*src[y1*sw + x1];

            // 再做垂直方向內插
            dst[y*dw + x] = (unsigned char)((1 - dy)*v0 + dy*v1);
        }
    }
}

// ---------------------- 顯示灰階影像 ----------------------
void showImageSDL(SDL_Renderer* ren, uint8_t* img, int w,int h){
    // 將灰階轉 RGB
    uint8_t* rgb=(uint8_t*)malloc(w*h*3);
    for(int i=0;i<w*h;i++){
        rgb[i*3+0]=img[i]; // B
        rgb[i*3+1]=img[i]; // G
        rgb[i*3+2]=img[i]; // R
    }

    // 建立 texture 並更新影像
    SDL_Texture* tex=SDL_CreateTexture(ren,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,w,h);
    SDL_UpdateTexture(tex,NULL,rgb,w*3);

    // 畫面顯示
    SDL_Rect dst={0,0,w,h};
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren,tex,NULL,&dst);
    SDL_RenderPresent(ren);

    SDL_DestroyTexture(tex);
    free(rgb);
}

// ---------------------- 等待視窗關閉 ----------------------
void waitCloseWindow() {
    SDL_Event e;
    for (;;) {
        if (SDL_WaitEvent(&e)) {
            if (e.type == SDL_QUIT) break; // 只在視窗被關掉才離開
        }
    }
}

// ---------------------- 主程式 ----------------------
int main() {
    const char* files[6] = {
        "images/baboon.bmp","images/boat.bmp","images/F16.bmp",
        "images/goldhill.raw","images/lena.raw","images/peppers.raw"
    };

    SDL_Init(SDL_INIT_VIDEO); // 初始化 SDL

    // 建立視窗與 Renderer
    SDL_Window* win=SDL_CreateWindow("Image Resize",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,512,512,
        SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);

    unsigned char input[IN_H][IN_W]; // 儲存原影像
    int cases[5][2]={{128,128},{32,32},{512,512},{1024,512},{256,512}}; // 縮放尺寸

    for(int f=0;f<6;f++){
        int ok=0;
        // 根據副檔名讀取 BMP 或 RAW
        if(strstr(files[f],".bmp")) ok=readBMPGray(files[f],input);
        else ok=readRAW(files[f],input);
        if(!ok) {printf("skip %s\n",files[f]); continue;}

        printf("\n=== %s ===\n",files[f]);
        for(int c=0;c<5;c++){
            int dw=cases[c][0],dh=cases[c][1];
            printf("Case %d: %dx%d\n",c+1,dw,dh);

            unsigned char* src=&input[0][0];
            unsigned char* tmp=(unsigned char*)malloc(dw*dh);  // 最近鄰結果
            unsigned char* tmp2=(unsigned char*)malloc(dw*dh); // 雙線性結果

            // 最近鄰縮放
            resizeNearest(src,IN_W,IN_H,tmp,dw,dh);
            showImageSDL(ren,tmp,dw,dh);
            printf("Nearest neighbor. Close window to continue...\n");
            waitCloseWindow();

            // 雙線性縮放
            resizeBilinear(src,IN_W,IN_H,tmp2,dw,dh);
            showImageSDL(ren,tmp2,dw,dh);
            printf("Bilinear. Close window to continue...\n");
            waitCloseWindow();

            free(tmp);free(tmp2);
        }
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
