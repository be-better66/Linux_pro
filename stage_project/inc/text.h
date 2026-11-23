#ifndef __TEXT_H__
#define __TEXT_H__

#include <stdint.h>

/*
 * 基于 stb_truetype 的中文文字绘制。
 * 需要开发板上存在 .ttf 字库文件（默认 simfang.ttf）。
 * 文字直接混合绘制到 LCD，不在内存里申请大位图。
 */

int  text_init(const char *ttf_path);
void text_exit(void);

/* 指定字号时，一行文字的高度（像素） */
int  text_height(int px);

/* 指定字号时，字符串的显示宽度（支持 UTF-8 中文） */
int  text_measure(const char *utf8, int px);

/* 在 (x, y) 绘制，y 为文字行顶部；支持 '\n' 换行 */
void text_draw(int x, int y, int px, const char *utf8, uint32_t rgb);

/* 以 cx 为中心水平居中绘制 */
void text_draw_center(int cx, int y, int px, const char *utf8, uint32_t rgb);

#endif
