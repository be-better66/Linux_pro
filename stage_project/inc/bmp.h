#ifndef __BMP_H__
#define __BMP_H__

#include <stdint.h>

/* 解码后的位图（0x00RRGGBB，行优先，左上角为原点） */
typedef struct {
    int width;
    int height;
    uint32_t *pixels;
} bmp_image_t;

/* 加载 24/32 位、未压缩的 BMP，失败返回 -1 */
int  bmp_load(const char *path, bmp_image_t *img);
void bmp_free(bmp_image_t *img);

/* 直接把图片画到屏幕 (x,y)，失败返回 -1 */
int  bmp_show(const char *path, int x, int y);

/*
 * 加载图片并居中放入 screen_w * screen_h 的缓冲区（不缩放，四周补黑）。
 * dst 至少 screen_w * screen_h 个 uint32_t。成功返回 0。
 */
int  bmp_load_fit(const char *path, int screen_w, int screen_h, uint32_t *dst);

#endif
