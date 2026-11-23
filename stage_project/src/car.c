#include "car.h"

#include "bmp.h"
#include "lcd.h"
#include "text.h"
#include "touch.h"
#include "car_photo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

car_state_t g_car;

/* 触摸调试：改成 0 可关闭串口日志 */
#define CAR_TOUCH_DEBUG 1

long long car_now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000LL + (long long)(tv.tv_usec / 1000);
}

int car_load_image(const char *path, bmp_image_t *img, int *have)
{
    if (img == NULL || have == NULL)
        return -1;
    if (*have && img->pixels != NULL)
        return 0;
    if (path == NULL || path[0] == 0)
        return -1;

    if (bmp_load(path, img) < 0) {
        *have = 0;
        return -1;
    }
    *have = 1;
    printf("[car] 图片加载: %s (%dx%d)\n", path, img->width, img->height);
    return 0;
}

/* ------------------------------------------------------------------ */
/* 启动动画                                                            */
/* ------------------------------------------------------------------ */

static void car_boot(void)
{
    const car_theme_t *t = car_theme();
    int i;

    lcd_clear(0);

    if (g_car.have_logo && g_car.logo.pixels != NULL) {
        car_blit_center(&g_car.logo, LCD_WIDTH, LCD_HEIGHT);
    } else {
        ui_text_center(400, 160, 46, "GEC AUTO", t->accent);
        ui_text_center(400, 225, 22, "智能座舱", t->text);
    }

    ui_text_center(400, 360, 14, "SMART COCKPIT SYSTEM", t->sub);
    lcd_fill_rect(250, 408, 300, 6, t->line);

    for (i = 0; i <= 20; i++) {
        lcd_fill_rect(250, 408, 300 * i / 20, 6, t->accent);
        usleep(50000);
    }
    sleep(1);
}

/* ------------------------------------------------------------------ */
/* 页面切换                                                            */
/* ------------------------------------------------------------------ */

static car_page_t next_page(car_page_t page)
{
    switch (page) {
    case CAR_PAGE_HOME:     return CAR_PAGE_NAV;
    case CAR_PAGE_NAV:      return CAR_PAGE_DASH;
    case CAR_PAGE_DASH:     return CAR_PAGE_MUSIC;
    case CAR_PAGE_MUSIC:    return CAR_PAGE_PHOTO;
    case CAR_PAGE_PHOTO:    return CAR_PAGE_PHONE;
    case CAR_PAGE_PHONE:    return CAR_PAGE_SETTINGS;
    case CAR_PAGE_SETTINGS: return CAR_PAGE_HOME;
    case CAR_PAGE_REVERSE:  return CAR_PAGE_DASH;
    default:                return CAR_PAGE_HOME;
    }
}

static car_page_t prev_page(car_page_t page)
{
    switch (page) {
    case CAR_PAGE_HOME:     return CAR_PAGE_SETTINGS;
    case CAR_PAGE_NAV:      return CAR_PAGE_HOME;
    case CAR_PAGE_DASH:     return CAR_PAGE_NAV;
    case CAR_PAGE_MUSIC:    return CAR_PAGE_DASH;
    case CAR_PAGE_PHOTO:    return CAR_PAGE_MUSIC;
    case CAR_PAGE_PHONE:    return CAR_PAGE_PHOTO;
    case CAR_PAGE_SETTINGS: return CAR_PAGE_PHONE;
    case CAR_PAGE_REVERSE:  return CAR_PAGE_DASH;
    default:                return CAR_PAGE_HOME;
    }
}

static void car_transition(const uint32_t *a, const uint32_t *b, int dir)
{
    uint32_t *fb = lcd_screen_buffer();
    int stride = lcd_stride();
    int step, y, x;

    if (fb == NULL || a == NULL || b == NULL || stride <= 0)
        return;

    for (step = 0; step <= 8; step++) {
        int off = LCD_WIDTH * step / 8;

        for (y = 0; y < LCD_HEIGHT; y++) {
            uint32_t *d = fb + (size_t)y * (size_t)stride;
            const uint32_t *pa = a + (size_t)y * LCD_WIDTH;
            const uint32_t *pb = b + (size_t)y * LCD_WIDTH;

            if (dir == 0) {
                for (x = 0; x < LCD_WIDTH - off; x++)
                    d[x] = pa[x + off];
                for (; x < LCD_WIDTH; x++)
                    d[x] = pb[x - (LCD_WIDTH - off)];
            } else {
                for (x = 0; x < off; x++)
                    d[x] = pb[x];
                for (; x < LCD_WIDTH; x++)
                    d[x] = pa[x - off];
            }
        }
        usleep(12000);
    }
}

void car_goto(car_page_t page)
{
    car_page_t old;

    if (page < 0 || page >= CAR_PAGE_COUNT)
        return;
    if (page == g_car.page)
        return;

    old = g_car.page;

    if (old == CAR_PAGE_PHOTO && page != CAR_PAGE_PHOTO)
        car_photo_reset();

    if (g_car.prev != NULL && g_car.next != NULL)
        lcd_snapshot(g_car.prev);       /* 先抓当前屏幕 */

    g_car.page = page;

    if (g_car.prev != NULL && g_car.next != NULL) {
        /*
         * 关键：新页面先画到离屏 next，不能画到屏幕上，
         * 否则转场开始前会闪一下新页面。
         */
        lcd_set_target(g_car.next);
        car_page_enter(page);
        car_page_render();
        lcd_set_target(NULL);

        if (g_car.cfg.page_anim)
            car_transition(g_car.prev, g_car.next, (page > old) ? 0 : 1);
        else
            lcd_blit(0, 0, g_car.next, LCD_WIDTH, LCD_HEIGHT);
    } else {
        car_page_enter(page);
        car_page_render();
    }

    g_car.last_clock = car_now_ms();
}

void car_rebuild(void)
{
    if (g_car.next != NULL) {
        lcd_set_target(g_car.next);
        car_page_enter(g_car.page);
        car_page_render();
        lcd_set_target(NULL);
        lcd_blit(0, 0, g_car.next, LCD_WIDTH, LCD_HEIGHT);
    } else {
        car_page_enter(g_car.page);
        car_page_render();
    }
}

void car_back_home(void)
{
    if (g_car.page == CAR_PAGE_PHOTO && car_photo_view_active()) {
        car_photo_back_to_grid();
        return;
    }
    if (g_car.page == CAR_PAGE_REVERSE) {
        g_car.gear = 3;
        car_goto(CAR_PAGE_DASH);
    } else {
        car_goto(CAR_PAGE_HOME);
    }
}

void car_toggle_night(void)
{
    g_car.cfg.night = !g_car.cfg.night;
    car_rebuild();
}

/* ------------------------------------------------------------------ */
/* 触摸分派                                                            */
/* ------------------------------------------------------------------ */

static int moved_less_than(const touch_event_t *ev, int limit)
{
    int dx = ev->x - ev->start_x;
    int dy = ev->y - ev->start_y;
    return (abs(dx) < limit && abs(dy) < limit);
}

static void handle_touch(const touch_event_t *ev)
{
#if CAR_TOUCH_DEBUG
    printf("[touch] type=%d x=%d y=%d start=(%d,%d)\n",
           (int)ev->type, ev->x, ev->y, ev->start_x, ev->start_y);
#endif

    /* 返回键按“按下点”判断，手指轻微抖动也认 */
    if (g_car.page != CAR_PAGE_HOME &&
        ev->start_x < 80 && ev->start_y < 52 &&
        moved_less_than(ev, 100)) {
        car_back_home();
        return;
    }

    /*
     * 电阻屏点击时会有 20~70px 抖动。
     * 60~100px 的“小滑动”（含上下）按点击处理，
     * 避免电阻屏抖动把点击识别成切页或上下滑。
     */
    if ((ev->type == TOUCH_EVT_SWIPE_LEFT || ev->type == TOUCH_EVT_SWIPE_RIGHT ||
         ev->type == TOUCH_EVT_SWIPE_UP || ev->type == TOUCH_EVT_SWIPE_DOWN) &&
        moved_less_than(ev, 100)) {
        touch_event_t tap = *ev;
        tap.type = TOUCH_EVT_TAP;
        tap.x = ev->start_x;    /* 用按下点做命中，避免抬手抖动跑出按钮 */
        tap.y = ev->start_y;
        car_page_touch(&tap);
        return;
    }

    /* 相册全屏查看时，左右滑交给相册切上一张/下一张 */
    if ((ev->type == TOUCH_EVT_SWIPE_LEFT || ev->type == TOUCH_EVT_SWIPE_RIGHT) &&
        g_car.page == CAR_PAGE_PHOTO && car_photo_view_active()) {
        car_page_touch(ev);
        return;
    }

    if (ev->type == TOUCH_EVT_SWIPE_LEFT) {
        if (g_car.page == CAR_PAGE_MUSIC) {
            car_page_touch(ev);
            return;
        }
        if (g_car.page == CAR_PAGE_REVERSE) {
            g_car.gear = 3;
            car_goto(CAR_PAGE_DASH);
            return;
        }
        car_goto(next_page(g_car.page));
        return;
    }

    if (ev->type == TOUCH_EVT_SWIPE_RIGHT) {
        if (g_car.page == CAR_PAGE_MUSIC) {
            car_page_touch(ev);
            return;
        }
        if (g_car.page == CAR_PAGE_REVERSE) {
            g_car.gear = 3;
            car_goto(CAR_PAGE_DASH);
            return;
        }
        car_goto(prev_page(g_car.page));
        return;
    }

    if (ev->type == TOUCH_EVT_TAP) {
        touch_event_t tap = *ev;
        tap.x = ev->start_x;
        tap.y = ev->start_y;
        car_page_touch(&tap);
    } else {
        car_page_touch(ev);
    }
}

/* ------------------------------------------------------------------ */
/* 入口                                                                */
/* ------------------------------------------------------------------ */

static void car_load_all_images(void)
{
    car_load_image(g_car.cfg.bg_day, &g_car.bg_day, &g_car.have_bg_day);
    car_load_image(g_car.cfg.bg_night, &g_car.bg_night, &g_car.have_bg_night);
    car_load_image(g_car.cfg.logo, &g_car.logo, &g_car.have_logo);

    car_load_image(g_car.cfg.icon_nav, &g_car.icon_nav, &g_car.have_icon_nav);
    car_load_image(g_car.cfg.icon_dash, &g_car.icon_dash, &g_car.have_icon_dash);
    car_load_image(g_car.cfg.icon_music, &g_car.icon_music, &g_car.have_icon_music);
    car_load_image(g_car.cfg.icon_phone, &g_car.icon_phone, &g_car.have_icon_phone);
    car_load_image(g_car.cfg.icon_setting, &g_car.icon_setting, &g_car.have_icon_setting);
    car_load_image(g_car.cfg.icon_pic, &g_car.icon_pic, &g_car.have_icon_pic);

    car_load_image(g_car.cfg.icon_sun, &g_car.icon_sun, &g_car.have_icon_sun);
    car_load_image(g_car.cfg.icon_moon, &g_car.icon_moon, &g_car.have_icon_moon);
}

int car_run(void)
{
    int touch_ok;

    srand((unsigned)time(NULL));

    if (lcd_init() < 0)
        return 1;

    if (car_config_load("res/car/car.cfg", &g_car.cfg) < 0)
        printf("[car] 未找到 res/car/car.cfg，使用默认配置\n");

    printf("[car] 车型配置: %d 首歌, 主题=%s, 音量=%d\n",
           g_car.cfg.song_count,
           g_car.cfg.night ? "夜间" : "白天",
           g_car.cfg.volume);

    g_car.page = CAR_PAGE_HOME;
    g_car.speed = 0;
    g_car.rpm = 800;
    g_car.gear = 0;
    g_car.turn = 0;
    g_car.demo_on = g_car.cfg.demo;
    g_car.brightness = 80;
    g_car.music_index = 0;
    g_car.music_playing = 0;
    g_car.music_started = 0;
    g_car.music_pos = 0;
    g_car.phone_selected = 0;
    g_car.phone_calling = 0;

    if (text_init(g_car.cfg.font) < 0)
        printf("[car] 字库加载失败，文字将被跳过\n");

    touch_ok = (touch_init() == 0);
    if (!touch_ok)
        printf("[car] 触摸屏打开失败，仅自动演示\n");

    car_load_all_images();

    g_car.prev = (uint32_t *)malloc((size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
    g_car.next = (uint32_t *)malloc((size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));
    g_car.dash_cache = (uint32_t *)malloc((size_t)LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));

    if (g_car.prev == NULL || g_car.next == NULL || g_car.dash_cache == NULL)
        printf("[car] 内存不足，页面切换动画/仪表缓存将被禁用\n");

    atexit(car_music_stop);

    car_boot();

    g_car.page = CAR_PAGE_HOME;
    car_page_enter(CAR_PAGE_HOME);
    car_page_render();

    g_car.last_tick = car_now_ms();
    g_car.last_clock = g_car.last_tick;
    g_car.last_music = g_car.last_tick;
    g_car.last_phone = g_car.last_tick;
    g_car.demo_start = g_car.last_tick;

    for (;;) {
        touch_event_t ev;
        long long now;

        if (touch_ok) {
            int r = touch_wait(&ev, 20);
            if (r > 0)
                handle_touch(&ev);
            else if (r < 0)
                usleep(20000);
        } else {
            usleep(20000);
        }

        now = car_now_ms();
        if (now - g_car.last_tick >= 50) {
            g_car.last_tick = now;
            car_page_tick();
        }
    }

    return 0;
}
