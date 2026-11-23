#include "lcd.h"

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>

static int       g_fd = -1;
static uint32_t *g_screen = NULL;      /* 真实显存 */
static uint32_t *g_fb = NULL;          /* 当前绘制目标 */
static int       g_screen_stride = 0;
static int       g_w = 0;
static int       g_h = 0;
static int       g_stride = 0;
static int       g_bpp = 0;
static size_t    g_map_size = 0;

static int inside(int x, int y)
{
    return (g_fb != NULL) && (x >= 0) && (x < g_w) && (y >= 0) && (y < g_h);
}

int lcd_init(void)
{
    struct fb_var_screeninfo vi;
    struct fb_fix_screeninfo fi;
    size_t virt_h;
    size_t line_bytes;

    if (g_fb != NULL)
        return 0;

    g_fd = open("/dev/fb0", O_RDWR);
    if (g_fd < 0) {
        perror("[lcd] open /dev/fb0");
        return -1;
    }

    if (ioctl(g_fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        perror("[lcd] FBIOGET_VSCREENINFO");
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    memset(&fi, 0, sizeof(fi));
    if (ioctl(g_fd, FBIOGET_FSCREENINFO, &fi) < 0) {
        perror("[lcd] FBIOGET_FSCREENINFO");
        /* 拿不到 fixinfo 时按 800x480x4 处理 */
    }

    g_w = (int)vi.xres;
    g_h = (int)vi.yres;
    g_bpp = (int)vi.bits_per_pixel;

    if (g_w <= 0 || g_h <= 0) {
        fprintf(stderr, "[lcd] invalid resolution %dx%d\n", g_w, g_h);
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    if (g_bpp != 32) {
        fprintf(stderr, "[lcd] unsupported bpp: %d (need 32)\n", g_bpp);
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    line_bytes = (fi.line_length > 0) ? (size_t)fi.line_length
                                      : (size_t)g_w * (size_t)(g_bpp / 8);
    g_stride = (int)(line_bytes / (size_t)(g_bpp / 8));
    if (g_stride <= 0)
        g_stride = g_w;

    virt_h = (vi.yres_virtual > 0) ? (size_t)vi.yres_virtual : (size_t)g_h;
    g_map_size = line_bytes * virt_h;

    g_fb = (uint32_t *)mmap(NULL, g_map_size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, g_fd, 0);
    if (g_fb == MAP_FAILED) {
        perror("[lcd] mmap");
        g_fb = NULL;
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    g_screen = g_fb;
    g_screen_stride = g_stride;

    /* 固定显示第一屏，避免有些板子上电默认停在第二屏 */
    vi.xoffset = 0;
    vi.yoffset = 0;
    (void)ioctl(g_fd, FBIOPAN_DISPLAY, &vi);

    printf("[lcd] %dx%d, %d bpp, stride=%d, map=%lu bytes\n",
           g_w, g_h, g_bpp, g_stride, (unsigned long)g_map_size);
    return 0;
}

void lcd_exit(void)
{
    if (g_screen != NULL) {
        munmap(g_screen, g_map_size);
        g_screen = NULL;
    }
    g_fb = NULL;
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}

int lcd_width(void)  { return (g_w > 0) ? g_w : LCD_WIDTH; }
int lcd_height(void) { return (g_h > 0) ? g_h : LCD_HEIGHT; }
int lcd_stride(void) { return (g_stride > 0) ? g_stride : LCD_WIDTH; }
uint32_t *lcd_buffer(void) { return g_fb; }
uint32_t *lcd_screen_buffer(void) { return g_screen; }

void lcd_set_target(uint32_t *buffer)
{
    if (buffer == NULL) {
        g_fb = g_screen;
        g_stride = g_screen_stride;
    } else {
        g_fb = buffer;
        g_stride = LCD_WIDTH;
    }
    g_w = LCD_WIDTH;
    g_h = LCD_HEIGHT;
}

void lcd_pixel(int x, int y, uint32_t rgb)
{
    if (inside(x, y))
        g_fb[(size_t)y * (size_t)g_stride + (size_t)x] = rgb;
}

void lcd_blend_pixel(int x, int y, uint32_t rgb, int alpha)
{
    uint32_t dst;
    int r, g, b, dr, dg, db;
    int inv;

    if (!inside(x, y))
        return;
    if (alpha <= 0)
        return;
    if (alpha >= 255) {
        lcd_pixel(x, y, rgb);
        return;
    }

    inv = 255 - alpha;
    dst = g_fb[(size_t)y * (size_t)g_stride + (size_t)x];

    r = (int)((rgb >> 16) & 0xFF);
    g = (int)((rgb >> 8) & 0xFF);
    b = (int)(rgb & 0xFF);
    dr = (int)((dst >> 16) & 0xFF);
    dg = (int)((dst >> 8) & 0xFF);
    db = (int)(dst & 0xFF);

    r = (r * alpha + dr * inv) / 255;
    g = (g * alpha + dg * inv) / 255;
    b = (b * alpha + db * inv) / 255;

    g_fb[(size_t)y * (size_t)g_stride + (size_t)x] =
        ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void lcd_clear(uint32_t rgb)
{
    int y;
    if (g_fb == NULL)
        return;
    for (y = 0; y < g_h; y++) {
        uint32_t *row = g_fb + (size_t)y * (size_t)g_stride;
        int x;
        for (x = 0; x < g_w; x++)
            row[x] = rgb;
    }
}

void lcd_fill_rect(int x, int y, int w, int h, uint32_t rgb)
{
    int yy, xx;
    if (g_fb == NULL || w <= 0 || h <= 0)
        return;
    for (yy = y; yy < y + h; yy++) {
        uint32_t *row;
        if (yy < 0 || yy >= g_h)
            continue;
        row = g_fb + (size_t)yy * (size_t)g_stride;
        for (xx = x; xx < x + w; xx++) {
            if (xx >= 0 && xx < g_w)
                row[xx] = rgb;
        }
    }
}

void lcd_frame_rect(int x, int y, int w, int h, uint32_t rgb)
{
    if (w <= 0 || h <= 0)
        return;
    lcd_hline(x, y, w, rgb);
    lcd_hline(x, y + h - 1, w, rgb);
    lcd_vline(x, y, h, rgb);
    lcd_vline(x + w - 1, y, h, rgb);
}

void lcd_hline(int x, int y, int w, uint32_t rgb)
{
    int xx;
    if (g_fb == NULL || y < 0 || y >= g_h)
        return;
    for (xx = x; xx < x + w; xx++) {
        if (xx >= 0 && xx < g_w)
            g_fb[(size_t)y * (size_t)g_stride + (size_t)xx] = rgb;
    }
}

void lcd_vline(int x, int y, int h, uint32_t rgb)
{
    int yy;
    if (g_fb == NULL || x < 0 || x >= g_w)
        return;
    for (yy = y; yy < y + h; yy++) {
        if (yy >= 0 && yy < g_h)
            g_fb[(size_t)yy * (size_t)g_stride + (size_t)x] = rgb;
    }
}

void lcd_blit(int x, int y, const uint32_t *src, int w, int h)
{
    int sx = 0, sy = 0, dw = w, dh = h, row;
    if (g_fb == NULL || src == NULL || w <= 0 || h <= 0)
        return;

    if (x < 0) { sx = -x; dw -= sx; x = 0; }
    if (y < 0) { sy = -y; dh -= sy; y = 0; }
    if (x + dw > g_w) dw = g_w - x;
    if (y + dh > g_h) dh = g_h - y;
    if (dw <= 0 || dh <= 0)
        return;

    for (row = 0; row < dh; row++) {
        const uint32_t *s = src + (size_t)(sy + row) * (size_t)w + (size_t)sx;
        uint32_t *d = g_fb + (size_t)(y + row) * (size_t)g_stride + (size_t)x;
        memcpy(d, s, (size_t)dw * sizeof(uint32_t));
    }
}

void lcd_blit_alpha(int x, int y, const uint32_t *src, int w, int h, int alpha)
{
    int yy, xx;
    if (g_fb == NULL || src == NULL || w <= 0 || h <= 0)
        return;
    for (yy = 0; yy < h; yy++) {
        for (xx = 0; xx < w; xx++) {
            lcd_blend_pixel(x + xx, y + yy, src[(size_t)yy * (size_t)w + (size_t)xx], alpha);
        }
    }
}

void lcd_snapshot(uint32_t *dst)
{
    int y;
    if (dst == NULL)
        return;
    if (g_fb == NULL) {
        memset(dst, 0, (size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
        return;
    }
    for (y = 0; y < g_h; y++) {
        memcpy(dst + (size_t)y * (size_t)LCD_WIDTH,
               g_fb + (size_t)y * (size_t)g_stride,
               (size_t)g_w * sizeof(uint32_t));
    }
}

void lcd_present(void)
{
    /* 单缓冲直接写显存，这里保留接口 */
}
