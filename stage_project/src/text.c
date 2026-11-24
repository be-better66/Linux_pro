#include "text.h"
#include "lcd.h"
#include "truetype.h"
#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static stbtt_fontinfo g_info;
static unsigned char *g_ttf = NULL;
static int    g_loaded = 0;
static float  g_scale = 0.0f;
static int    g_px = 0;
static unsigned char *g_raster = NULL;
static size_t g_raster_cap = 0;

static unsigned int utf8_next(const unsigned char **pp)
{
    const unsigned char *p = *pp;
    unsigned int c = p[0];

    if (c < 0x80) {
        *pp = p + 1;
        return c;
    }
    if ((c & 0xE0) == 0xC0 && p[1] != 0) {
        *pp = p + 2;
        return ((c & 0x1F) << 6) | (p[1] & 0x3F);
    }
    if ((c & 0xF0) == 0xE0 && p[1] != 0 && p[2] != 0) {
        *pp = p + 3;
        return ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
    }
    if ((c & 0xF8) == 0xF0 && p[1] != 0 && p[2] != 0 && p[3] != 0) {
        *pp = p + 4;
        return ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12) |
               ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
    }
    *pp = p + 1;
    return '?';
}

int text_init(const char *ttf_path)
{
    int fd;
    off_t size;
    ssize_t got;

    if (ttf_path == NULL)
        return -1;

    fd = open(ttf_path, O_RDONLY);
    if (fd < 0) {
        log_error("[text] open font: %s", strerror(errno));
        return -1;
    }

    size = lseek(fd, 0, SEEK_END);
    if (size <= 0) {
        log_error("[text] font file is empty");
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    g_ttf = (unsigned char *)malloc((size_t)size);
    if (g_ttf == NULL) {
        log_error("[text] out of memory for font");
        close(fd);
        return -1;
    }

    got = read(fd, g_ttf, (size_t)size);
    close(fd);
    if (got != (ssize_t)size) {
        log_error("[text] read font failed");
        free(g_ttf);
        g_ttf = NULL;
        return -1;
    }

    if (stbtt_InitFont(&g_info, g_ttf, 0) <= 0) {
        log_error("[text] stbtt_InitFont failed");
        free(g_ttf);
        g_ttf = NULL;
        return -1;
    }

    g_loaded = 1;
    g_px = 0;
    log_info("[text] font loaded: %s (%ld bytes)", ttf_path, (long)size);
    return 0;
}

void text_exit(void)
{
    free(g_ttf);
    g_ttf = NULL;
    free(g_raster);
    g_raster = NULL;
    g_raster_cap = 0;
    g_loaded = 0;
}

static void set_size(int px)
{
    if (px < 8)
        px = 8;
    if (px > 96)
        px = 96;
    if (px != g_px) {
        g_scale = stbtt_ScaleForPixelHeight(&g_info, (float)px);
        g_px = px;
    }
}

int text_height(int px)
{
    int ascent, descent, line_gap;
    if (!g_loaded)
        return px;
    set_size(px);
    stbtt_GetFontVMetrics(&g_info, &ascent, &descent, &line_gap);
    return (int)((float)(ascent - descent) * g_scale + 0.5f);
}

int text_measure(const char *utf8, int px)
{
    const unsigned char *p;
    int max_w = 0, line_w = 0;

    if (!g_loaded || utf8 == NULL)
        return 0;

    set_size(px);
    p = (const unsigned char *)utf8;

    while (*p != 0) {
        unsigned int cp = utf8_next(&p);
        int advance = 0;

        if (cp == '\n') {
            if (line_w > max_w)
                max_w = line_w;
            line_w = 0;
            continue;
        }
        stbtt_GetCodepointHMetrics(&g_info, (int)cp, &advance, NULL);
        line_w += (int)((float)advance * g_scale + 0.5f);
    }

    if (line_w > max_w)
        max_w = line_w;
    return max_w;
}

void text_draw(int x, int y, int px, const char *utf8, uint32_t rgb)
{
    const unsigned char *p;
    int ascent, descent, line_gap;
    int ascent_px, line_h, sx = 0, sy = 0;

    if (!g_loaded || utf8 == NULL)
        return;

    set_size(px);
    stbtt_GetFontVMetrics(&g_info, &ascent, &descent, &line_gap);
    ascent_px = (int)((float)ascent * g_scale);
    line_h = (int)((float)(ascent - descent + line_gap) * g_scale + 0.5f);
    if (line_h <= 0)
        line_h = px;

    p = (const unsigned char *)utf8;

    while (*p != 0) {
        unsigned int cp = utf8_next(&p);
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        int cw, ch, adv = 0;

        if (cp == '\n') {
            sx = 0;
            sy += line_h;
            continue;
        }

        stbtt_GetCodepointBitmapBox(&g_info, (int)cp, g_scale, g_scale,
                                    &x0, &y0, &x1, &y1);
        cw = x1 - x0;
        ch = y1 - y0;

        if (cw > 0 && ch > 0) {
            size_t need = (size_t)cw * (size_t)ch;
            if (need > g_raster_cap) {
                unsigned char *tmp = (unsigned char *)realloc(g_raster, need);
                if (tmp != NULL) {
                    g_raster = tmp;
                    g_raster_cap = need;
                } else {
                    need = 0;
                }
            }
            if (need > 0) {
                int gx = x + sx + x0;
                int gy = y + sy + ascent_px + y0;
                int iy, ix;

                stbtt_MakeCodepointBitmap(&g_info, g_raster, cw, ch, cw,
                                          g_scale, g_scale, (int)cp);
                for (iy = 0; iy < ch; iy++) {
                    for (ix = 0; ix < cw; ix++) {
                        int a = (int)g_raster[(size_t)iy * (size_t)cw + (size_t)ix];
                        if (a > 0)
                            lcd_blend_pixel(gx + ix, gy + iy, rgb, a);
                    }
                }
            }
        }

        stbtt_GetCodepointHMetrics(&g_info, (int)cp, &adv, NULL);
        sx += (int)((float)adv * g_scale + 0.5f);
    }
}

void text_draw_center(int cx, int y, int px, const char *utf8, uint32_t rgb)
{
    int w = text_measure(utf8, px);
    text_draw(cx - w / 2, y, px, utf8, rgb);
}
