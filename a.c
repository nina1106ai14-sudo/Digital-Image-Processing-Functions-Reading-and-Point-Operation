#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define SDL_MAIN_HANDLED
#include "inc/SDL.h"  // 引入 SDL2
#undef main

#define IN_W 512  // 影像寬度
#define IN_H 512  // 影像高度

#pragma pack(1)  // 設定結構體一個位元組對齊，避免 BMP 讀取錯誤
typedef struct {
    unsigned short bfType;       // 檔案類型
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
    unsigned int biSizeImage;     // 影像大小
    int biXPelsPerMeter;          // 水平解析度
    int biYPelsPerMeter;          // 垂直解析度
    unsigned int biClrUsed;       // 調色盤顏色數量
    unsigned int biClrImportant;  // 重要顏色數量
} BITMAPINFOHEADER;
#pragma pack()

// 讀取 8-bit 灰階 BMP 影像
int readBMPGray(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {  // 檢查檔案是否開啟成功
        perror(filename);
        memset(image, 0, IN_H * IN_W);
        return 0;
    }

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    fread(&fileHeader, sizeof(fileHeader), 1, fp);
    fread(&infoHeader, sizeof(infoHeader), 1, fp);

    // 檢查影像尺寸與位元深度是否符合要求
    if (infoHeader.biWidth != IN_W || infoHeader.biHeight != IN_H || infoHeader.biBitCount != 8) {
        printf("BMP bit count not supported (only 8-bit) or size mismatch: %s\n", filename);
        fclose(fp);
        memset(image, 0, IN_H * IN_W);
        return 0;
    }

    // 讀取調色盤
    unsigned char palette[256][4];
    fread(palette, sizeof(unsigned char), 256 * 4, fp);

    // 每行資料 4-byte 對齊
    int row_padded = (IN_W + 3) & ~3;
    unsigned char row[row_padded];

    // 讀取每行像素，BMP 從下往上存
    for (int y = 0; y < IN_H; y++) {
        fread(row, 1, row_padded, fp);
        for (int x = 0; x < IN_W; x++) {
            unsigned char idx = row[x];
            // 調色盤 BGR 平均作為灰階值
            unsigned char gray = (palette[idx][0] + palette[idx][1] + palette[idx][2]) / 3;
            image[IN_H - 1 - y][x] = gray;
        }
    }

    fclose(fp);
    return 1;
}

// 讀取 RAW 影像 (純像素資料)
int readRAW(const char* filename, unsigned char image[IN_H][IN_W]) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) { perror(filename); memset(image,0,IN_H*IN_W); return 0; }
    size_t readBytes = fread(image, sizeof(unsigned char), IN_W * IN_H, fp);
    fclose(fp);

    if (readBytes != IN_W * IN_H) { // 檢查是否讀取完整
        printf("RAW read failed (read %zu bytes, expected %d)\n", readBytes, IN_W * IN_H);
        memset(image, 0, IN_H * IN_W);
        return 0;
    }
    return 1;
}

// 顯示影像，只有手動關閉視窗才會返回
void showImageSDL(unsigned char image[IN_H][IN_W], int w, int h, const char* title) {
    SDL_Window* win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STATIC, w, h);

    // 將灰階影像轉為 RGB
    unsigned char* rgb = (unsigned char*)malloc(w * h * 3);
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++) {
            rgb[(y * w + x) * 3 + 0] = image[y][x]; // B
            rgb[(y * w + x) * 3 + 1] = image[y][x]; // G
            rgb[(y * w + x) * 3 + 2] = image[y][x]; // R
        }

    SDL_UpdateTexture(tex, NULL, rgb, w * 3);

    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, tex, NULL, NULL);
    SDL_RenderPresent(ren);

    free(rgb);

    // 只在視窗關閉事件 SDL_QUIT 時返回
    SDL_Event e;
    for (;;) {
        if (SDL_WaitEvent(&e)) {
            if (e.type == SDL_QUIT) break; // 只在視窗被關掉時才跳出
            // 其他事件 (鍵盤/滑鼠/右鍵) 都忽略
        }
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
}

// 列印影像中心 10x10 像素值
void printCenter(unsigned char image[IN_H][IN_W]) {
    int startX = IN_W / 2 - 5, startY = IN_H / 2 - 5;
    printf("\nCenter 10x10 pixel values:\n");
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++)
            printf("%3d ", image[startY + y][startX + x]);
        printf("\n");
    }
}

int main() {
    const char* files[6] = {
        "images/baboon.bmp", "images/boat.bmp", "images/F16.bmp",
        "images/goldhill.raw", "images/lena.raw", "images/peppers.raw"
    };

    unsigned char input[IN_H][IN_W];

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return -1;
    }

    for (int i = 0; i < 6; i++) {
        printf("\n--- Processing %s ---\n", files[i]);

        int success = 0;
        if (strstr(files[i], ".bmp")) success = readBMPGray(files[i], input);
        else if (strstr(files[i], ".raw")) success = readRAW(files[i], input);

        if (!success) {
            printf("Skipping file due to read error: %s\n", files[i]);
            continue;
        }

        printCenter(input);                     // 列印影像中心 10x10 像素值
        showImageSDL(input, IN_W, IN_H, files[i]); // 顯示影像，手動關閉視窗才會跳下一張
    }

    SDL_Quit();
    printf("\nAll processing done. Exiting...\n");
    return 0;
}
