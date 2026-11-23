#ifndef __CAR_H__
#define __CAR_H__

#include <stdint.h>

#include "bmp.h"
#include "touch.h"
#include "mp3.h"

#define CAR_MAX_SONGS 8
#define CAR_PATH_LEN  256
#define CAR_NAME_LEN  96

typedef enum {
    CAR_PAGE_HOME = 0,
    CAR_PAGE_NAV,
    CAR_PAGE_DASH,
    CAR_PAGE_MUSIC,
    CAR_PAGE_PHOTO,
    CAR_PAGE_PHONE,
    CAR_PAGE_SETTINGS,
    CAR_PAGE_REVERSE,
    CAR_PAGE_COUNT
} car_page_t;

typedef struct {
    uint32_t bg1;
    uint32_t bg2;
    uint32_t card;
    uint32_t card_hi;
    uint32_t line;
    uint32_t text;
    uint32_t sub;
    uint32_t accent;
    uint32_t danger;
    uint32_t ok;
} car_theme_t;

typedef struct {
    int night;
    int volume;
    int temperature;
    int demo;
    int audio;            /* 0 = 纯模拟，1 = 调用 madplay 出声 */
    int page_anim;        /* 1 = 页面滑动转场，0 = 直接切换（默认，避免闪屏） */

    char font[CAR_PATH_LEN];
    char logo[CAR_PATH_LEN];
    char bg_day[CAR_PATH_LEN];
    char bg_night[CAR_PATH_LEN];
    char icon_nav[CAR_PATH_LEN];
    char icon_dash[CAR_PATH_LEN];
    char icon_music[CAR_PATH_LEN];
    char icon_phone[CAR_PATH_LEN];
    char icon_setting[CAR_PATH_LEN];
    char icon_pic[CAR_PATH_LEN];
    char icon_sun[CAR_PATH_LEN];
    char icon_moon[CAR_PATH_LEN];
    char dash_bg[CAR_PATH_LEN];
    char album[CAR_PATH_LEN];
    char photo_dir[CAR_PATH_LEN];
    char reverse[CAR_PATH_LEN];

    char music[CAR_MAX_SONGS][CAR_PATH_LEN];
    char song_title[CAR_MAX_SONGS][CAR_NAME_LEN];
    char song_artist[CAR_MAX_SONGS][CAR_NAME_LEN];
    int  song_length[CAR_MAX_SONGS];
    int  song_count;
} car_config_t;

typedef struct {
    car_config_t cfg;
    car_page_t   page;

    int speed;              /* km/h */
    int rpm;                /* r/min */
    int gear;               /* 0 P, 1 R, 2 N, 3 D */
    int turn;               /* 0 off, 1 left, 2 right, 3 hazard */
    int demo_on;
    int brightness;

    int music_index;
    int music_playing;
    int music_started;
    int music_pos;          /* seconds */

    int phone_selected;
    int phone_calling;

    long long demo_start;
    long long last_tick;
    long long last_clock;
    long long last_music;
    long long last_phone;

    bmp_image_t bg_day;
    bmp_image_t bg_night;
    bmp_image_t logo;
    bmp_image_t icon_nav;
    bmp_image_t icon_dash;
    bmp_image_t icon_music;
    bmp_image_t icon_phone;
    bmp_image_t icon_setting;
    bmp_image_t icon_pic;
    bmp_image_t icon_sun;
    bmp_image_t icon_moon;
    bmp_image_t dash_bg;
    bmp_image_t album;
    bmp_image_t reverse;

    int have_bg_day;
    int have_bg_night;
    int have_logo;
    int have_icon_nav;
    int have_icon_dash;
    int have_icon_music;
    int have_icon_phone;
    int have_icon_setting;
    int have_icon_pic;
    int have_icon_sun;
    int have_icon_moon;
    int have_dash_bg;
    int have_album;
    int have_reverse;

    uint32_t *dash_cache;
    uint32_t *prev;
    uint32_t *next;
} car_state_t;

extern car_state_t g_car;

/* ---------------- car_cfg.c ---------------- */
void car_config_defaults(car_config_t *cfg);
int  car_config_load(const char *path, car_config_t *cfg);

/* ---------------- car_ui.c ---------------- */
const car_theme_t *car_theme(void);

void ui_gradient_v(int x, int y, int w, int h, uint32_t c1, uint32_t c2);
void ui_round_rect(int x, int y, int w, int h, int r, uint32_t color);
void ui_card(int x, int y, int w, int h, int r, uint32_t fill, uint32_t line);
void ui_circle(int cx, int cy, int r, uint32_t color);
void ui_circle_frame(int cx, int cy, int r, int t, uint32_t color);
void ui_arc(int cx, int cy, int r, int a0, int a1, uint32_t color, int t);
void ui_line(int x0, int y0, int x1, int y1, uint32_t color, int t);
void ui_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);

void ui_text_right(int right, int y, int px, const char *text, uint32_t color);
void ui_text_center(int cx, int y, int px, const char *text, uint32_t color);

void ui_draw_icon(int x, int y, int size, const bmp_image_t *img, int have,
                  const char *fallback, uint32_t fg, uint32_t bg);

void car_draw_status(void);
void car_draw_hint(const char *text);
void car_draw_bg(void);

const char *ui_page_title(car_page_t page);
const char *ui_weekday_cn(int wday);

/* ---------------- car_pages.c ---------------- */
void car_page_enter(car_page_t page);
void car_page_render(void);
void car_page_touch(const touch_event_t *ev);
void car_page_tick(void);
void car_page_dash_frame(void);
void car_page_music_progress(void);

/* ---------------- car.c ---------------- */
int  car_run(void);
long long car_now_ms(void);
void car_goto(car_page_t page);
void car_back_home(void);
void car_rebuild(void);
void car_toggle_night(void);

int  car_load_image(const char *path, bmp_image_t *img, int *have);
void car_blit_center(const bmp_image_t *img, int sw, int sh);

#endif
