#include "car.h"

#include "lcd.h"
#include "text.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PI 3.14159265358979323846f

static const car_theme_t g_day_theme = {
    0x000B1B2E, 0x00143856, 0x001A2E48, 0x00244466,
    0x00305070, 0x00FFFFFF, 0x0098B8D8, 0x0000C2FF,
    0x00FF5252, 0x0032D074
};

static const car_theme_t g_night_theme = {
    0x0003050A, 0x000A1624, 0x00101828, 0x001A2C44,
    0x00243A54, 0x00E8F0FF, 0x007090B0, 0x0000A8E0,
    0x00E04040, 0x0020B060
};

const car_theme_t *car_theme(void)
{
    return g_car.cfg.night ? &g_night_theme : &g_day_theme;
}

const char *ui_weekday_cn(int wday)
{
    static const char *names[7] = {
        "周日", "周一", "周二", "周三", "周四", "周五", "周六"
    };
    if (wday < 0 || wday > 6)
        return "";
    return names[wday];
}

const char *ui_page_title(car_page_t page)
{
    switch (page) {
    case CAR_PAGE_HOME:     return "智能座舱";
    case CAR_PAGE_NAV:      return "导航";
    case CAR_PAGE_DASH:     return "仪表";
    case CAR_PAGE_MUSIC:    return "音乐";
    case CAR_PAGE_PHOTO:    return "相册";
    case CAR_PAGE_PHONE:    return "电话";
    case CAR_PAGE_SETTINGS: return "设置";
    case CAR_PAGE_REVERSE:  return "倒车影像";
    default:                return "";
    }
}

void ui_gradient_v(int x, int y, int w, int h, uint32_t c1, uint32_t c2)
{
    int i;
    int r1 = (int)((c1 >> 16) & 0xFF), g1 = (int)((c1 >> 8) & 0xFF), b1 = (int)(c1 & 0xFF);
    int r2 = (int)((c2 >> 16) & 0xFF), g2 = (int)((c2 >> 8) & 0xFF), b2 = (int)(c2 & 0xFF);

    if (h <= 0)
        return;
    for (i = 0; i < h; i++) {
        int t = (h == 1) ? 0 : (i * 255 / (h - 1));
        int r = (r1 * (255 - t) + r2 * t) / 255;
        int g = (g1 * (255 - t) + g2 * t) / 255;
        int b = (b1 * (255 - t) + b2 * t) / 255;
        lcd_hline(x, y + i, w, ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b);
    }
}

void ui_circle(int cx, int cy, int r, uint32_t color)
{
    int dy;
    if (r <= 0) {
        lcd_pixel(cx, cy, color);
        return;
    }
    for (dy = -r; dy <= r; dy++) {
        int dx = (int)sqrtf((float)(r * r - dy * dy));
        int y = cy + dy;
        int x0 = cx - dx;
        int x1 = cx + dx;

        if (y < 0 || y >= LCD_HEIGHT)
            continue;
        if (x1 < 0 || x0 >= LCD_WIDTH)
            continue;
        if (x0 < 0)
            x0 = 0;
        if (x1 >= LCD_WIDTH)
            x1 = LCD_WIDTH - 1;
        lcd_hline(x0, y, x1 - x0 + 1, color);
    }
}

void ui_round_rect(int x, int y, int w, int h, int r, uint32_t color)
{
    if (w <= 0 || h <= 0)
        return;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;
    if (r <= 0) {
        lcd_fill_rect(x, y, w, h, color);
        return;
    }

    lcd_fill_rect(x + r, y, w - r * 2, h, color);
    lcd_fill_rect(x, y + r, r, h - r * 2, color);
    lcd_fill_rect(x + w - r, y + r, r, h - r * 2, color);
    ui_circle(x + r, y + r, r, color);
    ui_circle(x + w - r - 1, y + r, r, color);
    ui_circle(x + r, y + h - r - 1, r, color);
    ui_circle(x + w - r - 1, y + h - r - 1, r, color);
}

void ui_card(int x, int y, int w, int h, int r, uint32_t fill, uint32_t line)
{
    ui_round_rect(x, y, w, h, r, line);
    ui_round_rect(x + 1, y + 1, w - 2, h - 2, r - 1, fill);
}

void ui_arc(int cx, int cy, int r, int a0, int a1, uint32_t color, int t)
{
    int a;
    if (a1 < a0) {
        int tmp = a0;
        a0 = a1;
        a1 = tmp;
    }
    for (a = a0; a <= a1; a++) {
        float rad = (float)a * PI / 180.0f;
        int x = cx + (int)(cosf(rad) * (float)r);
        int y = cy + (int)(sinf(rad) * (float)r);
        if (t <= 1)
            lcd_pixel(x, y, color);
        else
            ui_circle(x, y, t / 2, color);
    }
}

void ui_circle_frame(int cx, int cy, int r, int t, uint32_t color)
{
    ui_arc(cx, cy, r, 0, 360, color, t);
}

void ui_line(int x0, int y0, int x1, int y1, uint32_t color, int t)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int rad = t / 2;

    for (;;) {
        if (t <= 1) {
            lcd_pixel(x0, y0, color);
        } else {
            int oy, ox;
            for (oy = -rad; oy <= rad; oy++) {
                for (ox = -rad; ox <= rad; ox++) {
                    if (ox * ox + oy * oy <= rad * rad + rad)
                        lcd_pixel(x0 + ox, y0 + oy, color);
                }
            }
        }
        if (x0 == x1 && y0 == y1)
            break;
        {
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx)  { err += dx; y0 += sy; }
        }
    }
}

void ui_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    int miny = y0, maxy = y0, y;
    int xs[3];

    if (y1 < miny) miny = y1;
    if (y2 < miny) miny = y2;
    if (y1 > maxy) maxy = y1;
    if (y2 > maxy) maxy = y2;

    for (y = miny; y <= maxy; y++) {
        int n = 0;
        int i;
        int px[3] = { x0, x1, x2 };
        int py[3] = { y0, y1, y2 };

        for (i = 0; i < 3; i++) {
            int j = (i + 1) % 3;
            if ((py[i] <= y && y < py[j]) || (py[j] <= y && y < py[i])) {
                int x = px[i] + (y - py[i]) * (px[j] - px[i]) / (py[j] - py[i]);
                xs[n++] = x;
            }
        }
        if (n >= 2) {
            int lo = xs[0], hi = xs[0];
            int k;
            for (k = 1; k < n; k++) {
                if (xs[k] < lo) lo = xs[k];
                if (xs[k] > hi) hi = xs[k];
            }
            lcd_hline(lo, y, hi - lo + 1, color);
        }
    }
}

void ui_text_right(int right, int y, int px, const char *text, uint32_t color)
{
    int w = text_measure(text, px);
    text_draw(right - w, y, px, text, color);
}

void ui_text_center(int cx, int y, int px, const char *text, uint32_t color)
{
    int w = text_measure(text, px);
    text_draw(cx - w / 2, y, px, text, color);
}

void ui_draw_icon(int x, int y, int size, const bmp_image_t *img, int have,
                  const char *fallback, uint32_t fg, uint32_t bg)
{
    ui_round_rect(x, y, size, size, 16, bg);

    if (have && img != NULL && img->pixels != NULL) {
        int dx = x + (size - img->width) / 2;
        int dy = y + (size - img->height) / 2;
        lcd_blit(dx, dy, img->pixels, img->width, img->height);
        return;
    }

    ui_circle(x + size / 2, y + size / 2, size / 3, bg);
    ui_text_center(x + size / 2, y + size / 2 - size / 4, size / 2,
                   fallback, fg);
}

void car_blit_center(const bmp_image_t *img, int sw, int sh)
{
    int x, y;
    if (img == NULL || img->pixels == NULL)
        return;
    x = (sw - img->width) / 2;
    y = (sh - img->height) / 2;
    lcd_blit(x, y, img->pixels, img->width, img->height);
}

void car_draw_bg(void)
{
    const car_theme_t *t = car_theme();

    lcd_clear(0);
    if (g_car.cfg.night && g_car.have_bg_night) {
        car_blit_center(&g_car.bg_night, LCD_WIDTH, LCD_HEIGHT);
    } else if (!g_car.cfg.night && g_car.have_bg_day) {
        car_blit_center(&g_car.bg_day, LCD_WIDTH, LCD_HEIGHT);
    } else {
        ui_gradient_v(0, 0, LCD_WIDTH, LCD_HEIGHT, t->bg1, t->bg2);
        ui_circle(700, 100, 160, t->bg2);
        ui_circle(110, 420, 120, t->bg2);
    }
}

void car_draw_status(void)
{
    const car_theme_t *t = car_theme();
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char buf[64];
    int is_home = (g_car.page == CAR_PAGE_HOME);
    int i;

    lcd_fill_rect(0, 0, LCD_WIDTH, 44, t->bg1);
    lcd_fill_rect(0, 44, LCD_WIDTH, 2, t->accent);

    if (!is_home) {
        ui_round_rect(6, 4, 50, 36, 10, t->card_hi);
        ui_triangle(16, 22, 36, 10, 36, 34, t->text);
        lcd_fill_rect(34, 18, 10, 8, t->text);
    } else {
        text_draw(20, 10, 20, "GEC AUTO", t->accent);
    }

    if (lt != NULL) {
        snprintf(buf, sizeof(buf), "%02d:%02d", lt->tm_hour, lt->tm_min);
        text_draw(is_home ? 150 : 70, 8, 20, buf, t->text);

        snprintf(buf, sizeof(buf), "%02d-%02d %s",
                 lt->tm_mon + 1, lt->tm_mday, ui_weekday_cn(lt->tm_wday));
        text_draw(is_home ? 150 : 70, 29, 11, buf, t->sub);
    }

    ui_text_center(480, 12, 18, ui_page_title(g_car.page), t->sub);

    snprintf(buf, sizeof(buf), "%dC", g_car.cfg.temperature);
    text_draw(590, 12, 16, buf, t->text);

    snprintf(buf, sizeof(buf), "VOL %d", g_car.cfg.volume);
    text_draw(650, 15, 12, buf, t->sub);

    for (i = 0; i < 4; i++) {
        int h = 4 + i * 3;
        lcd_fill_rect(762 + i * 8, 30 - h, 4, h, t->accent);
    }
}

void car_draw_hint(const char *text)
{
    const car_theme_t *t = car_theme();
    lcd_fill_rect(0, 432, LCD_WIDTH, 48, t->bg1);
    lcd_fill_rect(0, 432, LCD_WIDTH, 1, t->line);
    ui_text_center(400, 446, 13, text, t->sub);
}
