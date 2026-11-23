#include "car.h"

#include "lcd.h"
#include "text.h"
#include "car_photo.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358979323846f

#define STATUS_H 44
#define HINT_Y   432

/* ================================================================== */
/* 首页                                                                */
/* ================================================================== */

#define HOME_W  240
#define HOME_H  150

static const int home_x[6] = { 20, 280, 540, 20, 280, 540 };
static const int home_y[6] = { 80, 80, 80, 250, 250, 250 };
static const car_page_t home_target[6] = {
    CAR_PAGE_NAV, CAR_PAGE_DASH, CAR_PAGE_MUSIC,
    CAR_PAGE_PHONE, CAR_PAGE_PHOTO, CAR_PAGE_SETTINGS
};

static void home_card(int x, int y, const bmp_image_t *icon, int have,
                      const char *fallback, const char *title, const char *sub)
{
    const car_theme_t *t = car_theme();

    ui_card(x, y, HOME_W, HOME_H, 16, t->card, t->line);
    ui_draw_icon(x + 12, y + 27, 96, icon, have, fallback, t->accent, t->card_hi);
    text_draw(x + 116, y + 34, 22, title, t->text);
    text_draw(x + 116, y + 72, 10, sub, t->sub);
    ui_text_right(x + HOME_W - 16, y + 60, 20, ">", t->accent);
}

static void home_render(void)
{
    const car_theme_t *t = car_theme();

    car_draw_bg();

    home_card(home_x[0], home_y[0], &g_car.icon_nav, g_car.have_icon_nav,
              "导", "导航", "NAVIGATION");
    home_card(home_x[1], home_y[1], &g_car.icon_dash, g_car.have_icon_dash,
              "表", "仪表", "INSTRUMENT");
    home_card(home_x[2], home_y[2], &g_car.icon_music, g_car.have_icon_music,
              "乐", "音乐", "MEDIA PLAYER");
    home_card(home_x[3], home_y[3], &g_car.icon_phone, g_car.have_icon_phone,
              "话", "电话", "BLUETOOTH");
    home_card(home_x[4], home_y[4], &g_car.icon_pic, g_car.have_icon_pic,
              "相", "相册", "PHOTO ALBUM");
    home_card(home_x[5], home_y[5], &g_car.icon_setting, g_car.have_icon_setting,
              "设", "设置", "SETTINGS");

    car_draw_status();
    car_draw_hint("点击卡片进入 · 左右滑动切换页面 · 右下角切换昼夜");

    ui_card(710, 438, 72, 36, 10, t->card, t->line);
    if (g_car.cfg.night && g_car.have_icon_moon && g_car.icon_moon.pixels != NULL &&
        g_car.icon_moon.width <= 32 && g_car.icon_moon.height <= 32)
        lcd_blit(714, 446, g_car.icon_moon.pixels, g_car.icon_moon.width, g_car.icon_moon.height);
    else if (!g_car.cfg.night && g_car.have_icon_sun && g_car.icon_sun.pixels != NULL &&
             g_car.icon_sun.width <= 32 && g_car.icon_sun.height <= 32)
        lcd_blit(714, 446, g_car.icon_sun.pixels, g_car.icon_sun.width, g_car.icon_sun.height);
    ui_text_center(762, 448, 12, g_car.cfg.night ? "夜间" : "白天", t->text);
}

static void home_touch(const touch_event_t *ev)
{
    int x = ev->x;
    int y = ev->y;
    int i;

    if (ev->type != TOUCH_EVT_TAP)
        return;

    if (x >= 710 && x < 782 && y >= 438 && y < 474) {
        car_toggle_night();
        return;
    }

    for (i = 0; i < 6; i++) {
        if (x >= home_x[i] && x < home_x[i] + HOME_W &&
            y >= home_y[i] && y < home_y[i] + HOME_H) {
            car_goto(home_target[i]);
            return;
        }
    }
}

/* ================================================================== */
/* 导航                                                                */
/* ================================================================== */

static void nav_render(void)
{
    const car_theme_t *t = car_theme();
    int i;

    ui_gradient_v(0, STATUS_H, LCD_WIDTH, 388, t->bg1, t->bg2);

    for (i = 0; i < 8; i++) {
        ui_line(80 + i * 90, 60, 80 + i * 90, 420, t->line, 1);
        ui_line(20, 70 + i * 45, 780, 70 + i * 45, t->line, 1);
    }
    ui_line(20, 430, 300, 60, t->line, 2);

    ui_line(120, 400, 220, 330, t->accent, 6);
    ui_line(220, 330, 200, 250, t->accent, 6);
    ui_line(200, 250, 350, 210, t->accent, 6);
    ui_line(350, 210, 430, 170, t->accent, 6);
    ui_line(430, 170, 560, 150, t->accent, 6);

    ui_circle(120, 400, 8, t->ok);
    ui_circle(560, 150, 13, t->danger);
    ui_circle(560, 150, 5, t->text);
    ui_triangle(350, 202, 350, 218, 368, 210, t->accent);

    text_draw(576, 136, 12, "目的地", t->sub);

    ui_card(40, 320, 280, 80, 14, t->card, t->line);
    text_draw(60, 332, 18, "距目的地 3.2 km", t->text);
    text_draw(60, 362, 13, "预计 12 分钟 · 路况良好", t->sub);

    ui_card(600, 350, 160, 46, 12, t->accent, t->accent);
    ui_text_center(680, 360, 18, "结束导航", t->bg1);

    car_draw_status();
    car_draw_hint("导航中 · 点击“结束导航”返回主页 · 左右滑动切页");
}

static void nav_touch(const touch_event_t *ev)
{
    if (ev->type != TOUCH_EVT_TAP)
        return;
    if (ev->x >= 600 && ev->x < 760 && ev->y >= 350 && ev->y < 396)
        car_goto(CAR_PAGE_HOME);
}

/* ================================================================== */
/* 仪表盘                                                              */
/* ================================================================== */

#define DASH_CX 300
#define DASH_CY 240
#define DASH_R  150
#define RPM_CX  660
#define RPM_CY  290
#define RPM_R   92

static int gauge_px(int cx, int r, int vmax, int value)
{
    float a = 135.0f + 270.0f * (float)value / (float)vmax;
    return cx + (int)(cosf(a * PI / 180.0f) * (float)r);
}

static int gauge_py(int cy, int r, int vmax, int value)
{
    float a = 135.0f + 270.0f * (float)value / (float)vmax;
    return cy + (int)(sinf(a * PI / 180.0f) * (float)r);
}

static void gauge_ticks(int cx, int cy, int r, int vmax, int major,
                        int label_step, int div, int label_px)
{
    const car_theme_t *t = car_theme();
    int v;

    for (v = 0; v <= vmax; v += major) {
        int is_label = (v % label_step == 0);
        int x1 = gauge_px(cx, r - 5, vmax, v);
        int y1 = gauge_py(cy, r - 5, vmax, v);
        int x2 = gauge_px(cx, is_label ? r - 18 : r - 11, vmax, v);
        int y2 = gauge_py(cy, is_label ? r - 18 : r - 11, vmax, v);

        ui_line(x1, y1, x2, y2, is_label ? t->text : t->sub, is_label ? 2 : 1);

        if (is_label) {
            char buf[16];
            int lx = gauge_px(cx, r - 34, vmax, v);
            int ly = gauge_py(cy, r - 34, vmax, v);
            snprintf(buf, sizeof(buf), "%d", v / div);
            ui_text_center(lx, ly - 6, label_px, buf, t->sub);
        }
    }
}

static void draw_needle(int cx, int cy, int r, int vmax, int value, uint32_t color)
{
    int ex, ey;
    if (value < 0) value = 0;
    if (value > vmax) value = vmax;

    ex = gauge_px(cx, r - 26, vmax, value);
    ey = gauge_py(cy, r - 26, vmax, value);

    ui_line(cx, cy, ex, ey, color, 5);
    ui_circle(cx, cy, 11, color);
    ui_circle(cx, cy, 5, 0x00101828);
}

static void gear_boxes_static(void)
{
    const car_theme_t *t = car_theme();
    int i;
    for (i = 0; i < 4; i++) {
        ui_card(250 + i * 78, 392, 62, 32, 8, t->card, t->line);
        {
            char s[2];
            s[0] = "PRND"[i];
            s[1] = 0;
            ui_text_center(250 + i * 78 + 31, 400, 18, s, t->sub);
        }
    }
}

static void draw_turn_arrow(int x, int y, int dir, uint32_t color);

static void dash_static_render(void)
{
    const car_theme_t *t = car_theme();
    int i;

    if (g_car.have_dash_bg && g_car.dash_bg.pixels != NULL) {
        lcd_clear(0);
        car_blit_center(&g_car.dash_bg, LCD_WIDTH, LCD_HEIGHT);
    } else {
        ui_gradient_v(0, 0, LCD_WIDTH, LCD_HEIGHT, t->bg1, t->bg2);
    }

    ui_circle(DASH_CX, DASH_CY, DASH_R - 4, 0x0009121C);
    ui_circle_frame(DASH_CX, DASH_CY, DASH_R, 2, t->line);
    gauge_ticks(DASH_CX, DASH_CY, DASH_R, 240, 20, 40, 1, 12);
    ui_text_center(DASH_CX, 362, 12, "km/h", t->sub);

    ui_circle(RPM_CX, RPM_CY, RPM_R - 4, 0x0009121C);
    ui_circle_frame(RPM_CX, RPM_CY, RPM_R, 2, t->line);
    gauge_ticks(RPM_CX, RPM_CY, RPM_R, 80, 5, 10, 10, 10);
    ui_text_center(RPM_CX, RPM_CY + 58, 10, "x1000 r/min", t->sub);

    gear_boxes_static();

    text_draw(544, 390, 11, "油量", t->sub);
    lcd_fill_rect(590, 392, 150, 8, t->line);
    lcd_fill_rect(590, 392, 105, 8, t->ok);

    text_draw(544, 410, 11, "水温", t->sub);
    lcd_fill_rect(590, 412, 150, 8, t->line);
    lcd_fill_rect(590, 412, 78, 8, t->accent);

    text_draw(56, 400, 11, "ODO 12860 km", t->sub);

    draw_turn_arrow(14, 60, -1, t->line);
    draw_turn_arrow(748, 60, 1, t->line);

    for (i = 0; i < 10; i++) {
        int x = 60 + i * 20;
        lcd_pixel(x, 424, t->line);
    }

    car_draw_hint("点击表盘开关演示 · 点击挡位换挡 · 左右滑动切页");
}

static void draw_turn_arrow(int x, int y, int dir, uint32_t color)
{
    if (dir < 0) {
        ui_triangle(x, y, x + 18, y - 12, x + 18, y + 12, color);
        lcd_fill_rect(x + 16, y - 4, 16, 8, color);
    } else {
        ui_triangle(x + 18, y, x, y - 12, x, y + 12, color);
        lcd_fill_rect(x, y - 4, 16, 8, color);
    }
}

static void gear_highlight(void)
{
    const car_theme_t *t = car_theme();
    int i;
    for (i = 0; i < 4; i++) {
        uint32_t fill = (i == g_car.gear) ? t->accent : t->card_hi;
        char s[2];
        if (i != g_car.gear)
            continue;
        s[0] = "PRND"[i];
        s[1] = 0;
        ui_round_rect(252 + i * 78, 394, 58, 28, 6, fill);
        ui_text_center(250 + i * 78 + 31, 400, 18, s, t->bg1);
    }
}

void car_page_dash_frame(void)
{
    const car_theme_t *t = car_theme();
    char buf[32];

    if (g_car.dash_cache != NULL)
        lcd_blit(0, 0, g_car.dash_cache, LCD_WIDTH, LCD_HEIGHT);
    else
        dash_static_render();

    draw_needle(DASH_CX, DASH_CY, DASH_R, 240, g_car.speed, t->accent);
    draw_needle(RPM_CX, RPM_CY, RPM_R, 80, g_car.rpm / 100, t->danger);

    snprintf(buf, sizeof(buf), "%d", g_car.speed);
    ui_text_center(DASH_CX, 300, 54, buf, t->text);

    snprintf(buf, sizeof(buf), "%.1f", (double)g_car.rpm / 1000.0);
    ui_text_center(RPM_CX, RPM_CY + 34, 18, buf, t->text);

    gear_highlight();

    if (g_car.turn == 1 || g_car.turn == 3) {
        if (((car_now_ms() / 450) % 2) == 0)
            draw_turn_arrow(14, 60, -1, t->ok);
    }
    if (g_car.turn == 2 || g_car.turn == 3) {
        if (((car_now_ms() / 450) % 2) == 0)
            draw_turn_arrow(748, 60, 1, t->ok);
    }

    car_draw_status();
}

static void dash_touch(const touch_event_t *ev)
{
    int x = ev->x;
    int y = ev->y;
    int i;

    if (ev->type != TOUCH_EVT_TAP)
        return;

    if (x >= 100 && x < 500 && y >= 80 && y < 380) {
        g_car.demo_on = !g_car.demo_on;
        g_car.demo_start = car_now_ms();
        if (!g_car.demo_on) {
            g_car.speed = 0;
            g_car.rpm = 800;
            g_car.gear = 0;
        }
        return;
    }

    for (i = 0; i < 4; i++) {
        int gx = 250 + i * 78;
        if (x >= gx && x < gx + 62 && y >= 392 && y < 424) {
            g_car.gear = i;
            g_car.demo_on = 0;
            g_car.speed = 0;
            g_car.rpm = 800;
            return;
        }
    }

    if (x < 100 && y >= 46 && y < 100) {
        g_car.turn = (g_car.turn == 1) ? 0 : 1;
        return;
    }
    if (x > 700 && y >= 46 && y < 100) {
        g_car.turn = (g_car.turn == 2) ? 0 : 2;
        return;
    }
}

/* ================================================================== */
/* 音乐                                                                */
/* ================================================================== */

static void draw_note(int cx, int cy, uint32_t color)
{
    ui_circle(cx - 16, cy + 18, 12, color);
    ui_circle(cx + 16, cy + 10, 12, color);
    lcd_fill_rect(cx - 8, cy - 30, 4, 50, color);
    lcd_fill_rect(cx + 24, cy - 40, 4, 50, color);
    ui_line(cx - 6, cy - 30, cx + 26, cy - 40, color, 4);
    ui_line(cx - 6, cy - 18, cx + 26, cy - 28, color, 4);
}

static void draw_play_icon(int cx, int cy, int playing, uint32_t color)
{
    if (playing) {
        lcd_fill_rect(cx - 12, cy - 14, 8, 28, color);
        lcd_fill_rect(cx + 4, cy - 14, 8, 28, color);
    } else {
        ui_triangle(cx - 9, cy - 15, cx - 9, cy + 15, cx + 15, cy, color);
    }
}

static void draw_prev_icon(int cx, int cy, uint32_t color)
{
    lcd_fill_rect(cx - 18, cy - 12, 4, 24, color);
    ui_triangle(cx + 6, cy - 12, cx + 6, cy + 12, cx - 10, cy, color);
    ui_triangle(cx + 18, cy - 12, cx + 18, cy + 12, cx + 2, cy, color);
}

static void draw_next_icon(int cx, int cy, uint32_t color)
{
    lcd_fill_rect(cx + 14, cy - 12, 4, 24, color);
    ui_triangle(cx - 6, cy - 12, cx - 6, cy + 12, cx + 10, cy, color);
    ui_triangle(cx - 18, cy - 12, cx - 18, cy + 12, cx - 2, cy, color);
}

static void music_controls(void)
{
    const car_theme_t *t = car_theme();

    ui_circle(420, 320, 34, t->card);
    draw_prev_icon(420, 320, t->text);

    ui_circle(520, 320, 38, t->accent);
    draw_play_icon(520, 320, g_car.music_playing, t->bg1);

    ui_circle(620, 320, 34, t->card);
    draw_next_icon(620, 320, t->text);
}

static void music_render(void)
{
    const car_theme_t *t = car_theme();
    int idx = g_car.music_index;
    const char *title = "未知歌曲";
    const char *artist = "";
    int len = 240;
    int pos = g_car.music_pos;
    char buf[64];

    if (idx >= 0 && idx < g_car.cfg.song_count) {
        title = g_car.cfg.song_title[idx];
        artist = g_car.cfg.song_artist[idx];
        len = g_car.cfg.song_length[idx];
    }
    if (len <= 0) len = 240;
    if (pos > len) pos = len;

    ui_gradient_v(0, STATUS_H, LCD_WIDTH, 388, t->bg1, t->bg2);
    ui_circle(700, 120, 150, t->bg2);
    ui_circle(80, 390, 100, t->bg2);

    ui_card(40, 80, 300, 300, 18, t->card, t->line);
    if (g_car.have_album && g_car.album.pixels != NULL) {
        lcd_blit(40 + (300 - g_car.album.width) / 2,
                 80 + (300 - g_car.album.height) / 2,
                 g_car.album.pixels, g_car.album.width, g_car.album.height);
    } else {
        ui_gradient_v(42, 82, 296, 296, t->card_hi, t->bg2);
        draw_note(190, 230, t->accent);
    }

    text_draw(380, 92, 28, title, t->text);
    text_draw(380, 136, 16, artist, t->sub);
    snprintf(buf, sizeof(buf), "%02d", idx + 1);
    ui_text_right(760, 62, 14, buf, t->accent);

    lcd_fill_rect(380, 220, 360, 8, t->line);
    if (len > 0) {
        int w = 360 * pos / len;
        lcd_fill_rect(380, 220, w, 8, t->accent);
        ui_circle(380 + w, 224, 7, t->accent);
    }

    snprintf(buf, sizeof(buf), "%02d:%02d", pos / 60, pos % 60);
    text_draw(380, 238, 12, buf, t->sub);
    snprintf(buf, sizeof(buf), "%02d:%02d", len / 60, len % 60);
    ui_text_right(740, 238, 12, buf, t->sub);

    music_controls();

    text_draw(380, 380, 12, "音量", t->sub);
    lcd_fill_rect(430, 384, 220, 6, t->line);
    lcd_fill_rect(430, 384, 220 * g_car.cfg.volume / 30, 6, t->accent);
    ui_text_center(392, 376, 18, "-", t->text);
    ui_text_center(668, 376, 18, "+", t->text);

    car_draw_status();
    car_draw_hint("左右滑动切歌 · 点击进度条跳转 · 左上角返回");
}

void car_page_music_progress(void)
{
    music_render();
}

static void music_touch(const touch_event_t *ev)
{
    int x = ev->x;
    int y = ev->y;
    int len;
    int idx = g_car.music_index;

    if (ev->type == TOUCH_EVT_SWIPE_LEFT) {
        car_music_next();
        return;
    }
    if (ev->type == TOUCH_EVT_SWIPE_RIGHT) {
        car_music_prev();
        return;
    }
    if (ev->type != TOUCH_EVT_TAP)
        return;

    if (y >= 200 && y < 245 && x >= 380 && x < 740) {
        len = (idx >= 0 && idx < g_car.cfg.song_count) ? g_car.cfg.song_length[idx] : 240;
        if (len <= 0) len = 240;
        g_car.music_pos = (x - 380) * len / 360;
        g_car.last_music = car_now_ms();
        music_render();
        return;
    }

    if (x >= 380 && x < 460 && y >= 284 && y < 358) {
        car_music_prev();
        return;
    }
    if (x >= 480 && x < 560 && y >= 280 && y < 362) {
        car_music_toggle();
        music_render();
        return;
    }
    if (x >= 580 && x < 660 && y >= 284 && y < 358) {
        car_music_next();
        return;
    }

    if (x >= 372 && x < 412 && y >= 360 && y < 408) {
        if (g_car.cfg.volume > 0) g_car.cfg.volume--;
        music_render();
        return;
    }
    if (x >= 648 && x < 688 && y >= 360 && y < 408) {
        if (g_car.cfg.volume < 30) g_car.cfg.volume++;
        music_render();
        return;
    }
}

/* ================================================================== */
/* 电话                                                                */
/* ================================================================== */

static const char *g_contact_name[4] = { "张三", "李四", "王五", "赵六" };
static const char *g_contact_num[4]  = {
    "138 0013 8000", "139 0013 9000", "137 0013 7000", "136 0013 6000"
};

static void phone_render(void)
{
    const car_theme_t *t = car_theme();
    int i;

    ui_gradient_v(0, STATUS_H, LCD_WIDTH, 388, t->bg1, t->bg2);
    ui_circle(720, 100, 140, t->bg2);

    for (i = 0; i < 4; i++) {
        int y = 64 + i * 68;
        uint32_t line = (i == g_car.phone_selected) ? t->accent : t->line;
        ui_card(40, y, 720, 56, 14, t->card, line);

        ui_circle(76, y + 28, 20, t->card_hi);
        {
            char idx[2];
            idx[0] = (char)('1' + i);
            idx[1] = 0;
            ui_text_center(76, y + 18, 18, idx, t->accent);
        }
        text_draw(112, y + 8, 18, g_contact_name[i], t->text);
        text_draw(112, y + 32, 12, g_contact_num[i], t->sub);
        ui_text_right(738, y + 16, 14, "拨号 >", t->sub);
    }

    if (g_car.phone_calling) {
        ui_card(180, 120, 440, 250, 18, t->card, t->accent);
        ui_text_center(400, 146, 18, "正在呼叫", t->sub);
        ui_text_center(400, 184, 28, g_contact_name[g_car.phone_selected], t->text);
        ui_text_center(400, 224, 14, g_contact_num[g_car.phone_selected], t->sub);
        ui_card(300, 292, 200, 48, 12, t->danger, t->danger);
        ui_text_center(400, 304, 20, "挂断", t->text);
    } else {
        ui_card(300, 356, 200, 48, 12, t->accent, t->accent);
        ui_text_center(400, 368, 20, "呼叫", t->bg1);
    }

    car_draw_status();
    car_draw_hint("点击联系人选择 · 点击呼叫/挂断 · 左上角返回");
}

static void phone_touch(const touch_event_t *ev)
{
    int x = ev->x;
    int y = ev->y;
    int i;

    if (ev->type != TOUCH_EVT_TAP)
        return;

    if (g_car.phone_calling) {
        if (x >= 300 && x < 500 && y >= 292 && y < 340) {
            g_car.phone_calling = 0;
            phone_render();
        }
        return;
    }

    for (i = 0; i < 4; i++) {
        int cy = 64 + i * 68;
        if (x >= 40 && x < 760 && y >= cy && y < cy + 56) {
            g_car.phone_selected = i;
            phone_render();
            return;
        }
    }

    if (x >= 300 && x < 500 && y >= 356 && y < 404) {
        g_car.phone_calling = 1;
        g_car.last_phone = car_now_ms();
        phone_render();
    }
}

/* ================================================================== */
/* 设置                                                                */
/* ================================================================== */

static void settings_row(int y, const char *label, const char *value)
{
    const car_theme_t *t = car_theme();
    ui_card(40, y, 720, 56, 14, t->card, t->line);
    text_draw(64, y + 16, 18, label, t->text);
    ui_text_right(600, y + 18, 16, value, t->sub);
}

static void settings_render(void)
{
    const car_theme_t *t = car_theme();
    char buf[32];

    ui_gradient_v(0, STATUS_H, LCD_WIDTH, 388, t->bg1, t->bg2);
    ui_circle(700, 120, 150, t->bg2);

    settings_row(60, "主题", g_car.cfg.night ? "夜间" : "白天");

    settings_row(128, "亮度", "");
    snprintf(buf, sizeof(buf), "%d", g_car.brightness);
    ui_text_center(540, 146, 16, buf, t->accent);
    ui_text_center(650, 140, 26, "-", t->text);
    ui_text_center(710, 140, 26, "+", t->text);

    settings_row(196, "音量", "");
    snprintf(buf, sizeof(buf), "%d", g_car.cfg.volume);
    ui_text_center(540, 214, 16, buf, t->accent);
    ui_text_center(650, 208, 26, "-", t->text);
    ui_text_center(710, 208, 26, "+", t->text);

    settings_row(264, "仪表演示", g_car.demo_on ? "开" : "关");
    settings_row(332, "倒车影像", "进入 >");

    car_draw_status();
    car_draw_hint("点击整行切换主题/演示 · 点击 +/- 调节 · 左上角返回");
}

static void settings_touch(const touch_event_t *ev)
{
    int x = ev->x;
    int y = ev->y;

    if (ev->type != TOUCH_EVT_TAP)
        return;

    if (y >= 60 && y < 116) {
        car_toggle_night();
        return;
    }
    if (y >= 128 && y < 184) {
        if (x >= 620 && x < 680) {
            if (g_car.brightness >= 10) g_car.brightness -= 10;
            settings_render();
        } else if (x >= 680 && x < 760) {
            if (g_car.brightness <= 90) g_car.brightness += 10;
            settings_render();
        }
        return;
    }
    if (y >= 196 && y < 252) {
        if (x >= 620 && x < 680) {
            if (g_car.cfg.volume > 0) g_car.cfg.volume--;
            settings_render();
        } else if (x >= 680 && x < 760) {
            if (g_car.cfg.volume < 30) g_car.cfg.volume++;
            settings_render();
        }
        return;
    }
    if (y >= 264 && y < 320) {
        g_car.demo_on = !g_car.demo_on;
        settings_render();
        return;
    }
    if (y >= 332 && y < 388) {
        car_goto(CAR_PAGE_REVERSE);
        return;
    }
}

/* ================================================================== */
/* 倒车                                                                */
/* ================================================================== */

static void reverse_render(void)
{
    const car_theme_t *t = car_theme();

    lcd_clear(0);
    if (g_car.have_reverse && g_car.reverse.pixels != NULL)
        car_blit_center(&g_car.reverse, LCD_WIDTH, LCD_HEIGHT);
    else
        ui_gradient_v(0, 0, LCD_WIDTH, LCD_HEIGHT, t->bg1, t->bg2);

    ui_line(250, 470, 330, 250, t->ok, 3);
    ui_line(550, 470, 470, 250, t->ok, 3);
    ui_line(330, 250, 470, 250, t->ok, 3);

    ui_line(276, 400, 524, 400, t->danger, 3);
    ui_line(300, 330, 500, 330, 0x00FFC107, 3);
    ui_line(318, 285, 482, 285, 0x00FFC107, 3);

    ui_text_center(400, 60, 22, "请注意后方", t->text);
    ui_text_right(760, 60, 30, "R", t->danger);

    car_draw_status();
    car_draw_hint("倒车影像 · 点击任意位置返回");
}

/* ================================================================== */
/* 页面分派                                                            */
/* ================================================================== */

void car_page_enter(car_page_t page)
{
    switch (page) {
    case CAR_PAGE_DASH:
        car_load_image(g_car.cfg.dash_bg, &g_car.dash_bg, &g_car.have_dash_bg);
        if (g_car.dash_cache == NULL)
            g_car.dash_cache = (uint32_t *)malloc((size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
        g_car.demo_start = car_now_ms();
        dash_static_render();
        if (g_car.dash_cache != NULL)
            lcd_snapshot(g_car.dash_cache);
        break;

    case CAR_PAGE_MUSIC:
        car_load_image(g_car.cfg.album, &g_car.album, &g_car.have_album);
        g_car.last_music = car_now_ms();
        break;

    case CAR_PAGE_PHOTO:
        car_photo_enter();
        break;

    case CAR_PAGE_REVERSE:
        car_load_image(g_car.cfg.reverse, &g_car.reverse, &g_car.have_reverse);
        g_car.gear = 1;
        break;

    default:
        break;
    }
}

void car_page_render(void)
{
    switch (g_car.page) {
    case CAR_PAGE_HOME:     home_render();     break;
    case CAR_PAGE_NAV:      nav_render();      break;
    case CAR_PAGE_DASH:     car_page_dash_frame(); break;
    case CAR_PAGE_MUSIC:    music_render();    break;
    case CAR_PAGE_PHOTO:    car_photo_render(); break;
    case CAR_PAGE_PHONE:    phone_render();    break;
    case CAR_PAGE_SETTINGS: settings_render(); break;
    case CAR_PAGE_REVERSE:  reverse_render();  break;
    default:                home_render();     break;
    }
}

void car_page_touch(const touch_event_t *ev)
{
    switch (g_car.page) {
    case CAR_PAGE_HOME:     home_touch(ev);     break;
    case CAR_PAGE_NAV:      nav_touch(ev);      break;
    case CAR_PAGE_DASH:     dash_touch(ev);     break;
    case CAR_PAGE_MUSIC:    music_touch(ev);    break;
    case CAR_PAGE_PHOTO:    car_photo_touch(ev); break;
    case CAR_PAGE_PHONE:    phone_touch(ev);    break;
    case CAR_PAGE_SETTINGS: settings_touch(ev); break;
    case CAR_PAGE_REVERSE:
        if (ev->type == TOUCH_EVT_TAP) {
            g_car.gear = 3;
            car_goto(CAR_PAGE_DASH);
        }
        break;
    default: break;
    }
}

void car_page_tick(void)
{
    long long now = car_now_ms();

    if (g_car.page == CAR_PAGE_DASH) {
        if (g_car.demo_on) {
            float t = (float)(now - g_car.demo_start) / 1000.0f;
            g_car.speed = 58 + (int)(54.0f * sinf(t * 0.85f));
            if (g_car.speed < 0) g_car.speed = 0;
            if (g_car.speed > 240) g_car.speed = 240;
            g_car.rpm = 900 + g_car.speed * 32 + (int)(140.0f * sinf(t * 2.2f));
            if (g_car.rpm < 700) g_car.rpm = 700;
            if (g_car.rpm > 6800) g_car.rpm = 6800;
            g_car.gear = 3;
        } else {
            g_car.speed = 0;
            g_car.rpm = 800;
        }
        car_page_dash_frame();
        return;
    }

    if (g_car.page == CAR_PAGE_MUSIC && g_car.music_playing &&
        now - g_car.last_music >= 1000) {
        int idx = g_car.music_index;
        int len = (idx >= 0 && idx < g_car.cfg.song_count) ? g_car.cfg.song_length[idx] : 240;
        if (len <= 0) len = 240;
        g_car.last_music = now;
        g_car.music_pos++;
        if (g_car.music_pos >= len) {
            car_music_next();
        } else {
            car_page_music_progress();
        }
        return;
    }

    if (now - g_car.last_clock >= 1000) {
        g_car.last_clock = now;

        if (g_car.page == CAR_PAGE_PHONE && g_car.phone_calling) {
            g_car.last_phone = now;
            phone_render();
        } else {
            car_draw_status();
        }
    }
}
