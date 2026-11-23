#ifndef __LCD_H__
#define __LCD_H__

#include <stdint.h>

/*
 * GEC6818 液晶屏驱动（/dev/fb0）
 * 分辨率 800x480，色深 32bpp，颜色格式 0x00RRGGBB
 */

#define LCD_WIDTH   800
#define LCD_HEIGHT  480

#define LCD_BLACK   0x00000000u
#define LCD_WHITE   0x00FFFFFFu
#define LCD_RED     0x00FF0000u
#define LCD_GREEN   0x0000FF00u
#define LCD_BLUE    0x000000FFu
#define LCD_YELLOW  0x00FFFF00u
#define LCD_CYAN    0x0000FFFFu
#define LCD_MAGENTA 0x00FF00FFu
#define LCD_ORANGE  0x00FF8000u
#define LCD_GRAY    0x00808080u
#define LCD_DARK    0x00101828u

/* 初始化 / 释放液晶屏 */
int  lcd_init(void);
void lcd_exit(void);

/* 屏幕参数 */
int  lcd_width(void);
int  lcd_height(void);
int  lcd_stride(void);          /* 一行占多少个像素 */
uint32_t *lcd_buffer(void);     /* 当前绘制目标（默认显存） */
uint32_t *lcd_screen_buffer(void); /* 真实显存首地址 */
void lcd_set_target(uint32_t *buffer); /* NULL = 回到显存，非 NULL = 画到离屏缓冲 */

/* 基础绘图 */
void lcd_clear(uint32_t rgb);
void lcd_pixel(int x, int y, uint32_t rgb);
void lcd_blend_pixel(int x, int y, uint32_t rgb, int alpha);
void lcd_fill_rect(int x, int y, int w, int h, uint32_t rgb);
void lcd_frame_rect(int x, int y, int w, int h, uint32_t rgb);
void lcd_hline(int x, int y, int w, uint32_t rgb);
void lcd_vline(int x, int y, int h, uint32_t rgb);

/* 图片拷贝：src 为 800x480（或指定尺寸）的 0x00RRGGBB 数组 */
void lcd_blit(int x, int y, const uint32_t *src, int w, int h);
void lcd_blit_alpha(int x, int y, const uint32_t *src, int w, int h, int alpha);

/* 把当前屏幕内容保存到 dst（dst 至少 800*480 个 uint32_t） */
void lcd_snapshot(uint32_t *dst);

/* 提交显示（单缓冲时为占位函数，便于以后扩展双缓冲） */
void lcd_present(void);

#endif
