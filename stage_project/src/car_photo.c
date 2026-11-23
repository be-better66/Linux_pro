#include "car.h"
#include "car_photo.h"

#include "lcd.h"
#include "text.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define PHOTO_MAX 6

#define CARD_W   236
#define CARD_H   136
#define IMG_W    220
#define IMG_H    124
#define GAP_X    16
#define GAP_Y    14
#define GRID_X   30
#define GRID_Y   76

#define CLOSE_X  744
#define CLOSE_Y  52
#define CLOSE_W  40
#define CLOSE_H  40

static int  s_scanned = 0;
static int  s_view = 0;          /* 0 = 缩略图，1 = 全屏 */
static int  s_index = 0;
static int  s_count = 0;

static char s_name[PHOTO_MAX][64];
static bmp_image_t s_thumb[PHOTO_MAX];
static int  s_thumb_ok[PHOTO_MAX];

static bmp_image_t s_full;
static int  s_full_ok = 0;

static int name_cmp(const void *a, const void *b)
{
    const char *na = (const char *)a;
    const char *nb = (const char *)b;
    int va = atoi(na + 6);       /* photo_ 后面的编号 */
    int vb = atoi(nb + 6);

    if (va != vb)
        return va - vb;
    return strcmp(na, nb);
}

static void free_full(void)
{
    if (s_full.pixels != NULL)
        bmp_free(&s_full);
    memset(&s_full, 0, sizeof(s_full));
    s_full_ok = 0;
}

static void scan_photos(void)
{
    DIR *dir;
    struct dirent *ent;
    int n = 0;
    int i;

    if (s_scanned)
        return;
    s_scanned = 1;
    s_count = 0;

    dir = opendir(g_car.cfg.photo_dir);
    if (dir == NULL) {
        printf("[photo] 打不开目录: %s\n", g_car.cfg.photo_dir);
        return;
    }

    while ((ent = readdir(dir)) != NULL && n < PHOTO_MAX) {
        const char *name = ent->d_name;
        size_t len = strlen(name);

        if (strncmp(name, "photo_", 6) != 0)
            continue;
        if (len <= 10 || strcasecmp(name + len - 4, ".bmp") != 0)
            continue;

        snprintf(s_name[n], sizeof(s_name[n]), "%s", name);
        n++;
    }
    closedir(dir);

    qsort(s_name, (size_t)n, sizeof(s_name[0]), name_cmp);
    s_count = n;

    for (i = 0; i < s_count; i++) {
        char thumb[512];
        const char *tail = s_name[i] + 6;   /* 1.bmp */

        snprintf(thumb, sizeof(thumb), "%s/thumb_%s",
                 g_car.cfg.photo_dir, tail);
        memset(&s_thumb[i], 0, sizeof(s_thumb[i]));
        s_thumb_ok[i] = (bmp_load(thumb, &s_thumb[i]) == 0);
        if (!s_thumb_ok[i])
            printf("[photo] 缩略图缺失: %s\n", thumb);
    }

    printf("[photo] 共 %d 张，目录 %s\n", s_count, g_car.cfg.photo_dir);
}

static void load_full(int idx)
{
    char path[512];

    free_full();
    if (idx < 0 || idx >= s_count)
        return;

    snprintf(path, sizeof(path), "%s/%s",
             g_car.cfg.photo_dir, s_name[idx]);
    memset(&s_full, 0, sizeof(s_full));
    s_full_ok = (bmp_load(path, &s_full) == 0);
    if (!s_full_ok)
        printf("[photo] 图片加载失败: %s\n", path);
}

static void open_photo(int idx)
{
    if (idx < 0 || idx >= s_count)
        return;

    s_index = idx;
    s_view = 1;
    load_full(idx);
    car_rebuild();
}

static void step_photo(int delta)
{
    if (s_count <= 0)
        return;

    open_photo((s_index + delta + s_count) % s_count);
}

static void render_grid(void)
{
    const car_theme_t *t = car_theme();
    char buf[64];
    int i;

    car_draw_bg();

    text_draw(30, 50, 18, "我的相册", t->text);
    snprintf(buf, sizeof(buf), "共 %d 张 · 点击查看", s_count);
    text_draw(170, 54, 12, buf, t->sub);

    if (s_count == 0) {
        ui_text_center(400, 200, 28, "暂无照片", t->text);
        ui_text_center(400, 250, 14,
                       "把 photo_N.bmp 和 thumb_N.bmp 放到 res/photo", t->sub);
    }

    for (i = 0; i < s_count; i++) {
        int row = i / 3;
        int col = i % 3;
        int x = GRID_X + col * (CARD_W + GAP_X);
        int y = GRID_Y + row * (CARD_H + GAP_Y);

        ui_card(x, y, CARD_W, CARD_H, 14, t->card, t->line);

        if (s_thumb_ok[i] && s_thumb[i].pixels != NULL) {
            int ix = x + 8 + (IMG_W - s_thumb[i].width) / 2;
            int iy = y + 6 + (IMG_H - s_thumb[i].height) / 2;

            lcd_blit(ix, iy, s_thumb[i].pixels,
                     s_thumb[i].width, s_thumb[i].height);
        } else {
            lcd_fill_rect(x + 8, y + 6, IMG_W, IMG_H, t->card_hi);
            ui_text_center(x + CARD_W / 2, y + 58, 16, "PHOTO", t->sub);
        }

        /* 左上角序号 */
        ui_round_rect(x + 10, y + 8, 32, 22, 6, 0x00101828);
        snprintf(buf, sizeof(buf), "%d", i + 1);
        ui_text_center(x + 26, y + 12, 12, buf, t->text);
    }

    car_draw_status();
    car_draw_hint("点击照片查看 · 左右滑动切页 · 左上角返回");
}

static void render_view(void)
{
    const car_theme_t *t = car_theme();
    char hint[128];
    int ix, iy;

    lcd_clear(0x00000000);

    if (s_full_ok && s_full.pixels != NULL) {
        ix = (LCD_WIDTH - s_full.width) / 2;
        iy = (LCD_HEIGHT - s_full.height) / 2;
        if (ix < 0) ix = 0;
        if (iy < 0) iy = 0;
        lcd_blit(ix, iy, s_full.pixels, s_full.width, s_full.height);
    } else {
        ui_text_center(400, 200, 28, "图片加载失败", t->text);
        ui_text_center(400, 244, 14, "检查 res/photo 下的 photo_N.bmp", t->sub);
    }

    /* 左右切换箭头 */
    ui_card(8, 190, 44, 90, 12, t->card, t->line);
    ui_triangle(34, 235, 18, 210, 18, 260, t->text);
    ui_card(748, 190, 44, 90, 12, t->card, t->line);
    ui_triangle(766, 235, 782, 210, 782, 260, t->text);

    /* 右上角关闭，参考工程的“点右上角退出查看” */
    ui_card(CLOSE_X, CLOSE_Y, CLOSE_W, CLOSE_H, 10, t->card, t->line);
    ui_line(CLOSE_X + 12, CLOSE_Y + 12, CLOSE_X + 28, CLOSE_Y + 28, t->text, 2);
    ui_line(CLOSE_X + 28, CLOSE_Y + 12, CLOSE_X + 12, CLOSE_Y + 28, t->text, 2);

    car_draw_status();

    snprintf(hint, sizeof(hint), "%d / %d · 左右滑动或点两侧切换 · 右上角关闭",
             s_index + 1, s_count);
    car_draw_hint(hint);
}

void car_photo_enter(void)
{
    scan_photos();
    if (s_count <= 0)
        s_view = 0;
}

void car_photo_render(void)
{
    if (s_view && s_count > 0)
        render_view();
    else
        render_grid();
}

void car_photo_touch(const touch_event_t *ev)
{
    if (!s_view) {
        int i;

        if (ev->type != TOUCH_EVT_TAP)
            return;

        for (i = 0; i < s_count; i++) {
            int row = i / 3;
            int col = i % 3;
            int x = GRID_X + col * (CARD_W + GAP_X);
            int y = GRID_Y + row * (CARD_H + GAP_Y);

            if (ev->x >= x && ev->x < x + CARD_W &&
                ev->y >= y && ev->y < y + CARD_H) {
                open_photo(i);
                return;
            }
        }
        return;
    }

    if (ev->type == TOUCH_EVT_SWIPE_LEFT) {
        step_photo(1);
        return;
    }
    if (ev->type == TOUCH_EVT_SWIPE_RIGHT) {
        step_photo(-1);
        return;
    }
    if (ev->type != TOUCH_EVT_TAP)
        return;

    /* 右上角关闭，回缩略图 */
    if (ev->x >= CLOSE_X - 14 && ev->x < CLOSE_X + CLOSE_W &&
        ev->y >= CLOSE_Y - 10 && ev->y < CLOSE_Y + CLOSE_H) {
        car_photo_back_to_grid();
        return;
    }

    /* 参考工程：点左半屏上一张，右半屏下一张 */
    if (ev->x < LCD_WIDTH / 2)
        step_photo(-1);
    else
        step_photo(1);
}

int car_photo_view_active(void)
{
    return s_view && s_count > 0;
}

void car_photo_back_to_grid(void)
{
    if (!s_view)
        return;

    s_view = 0;
    free_full();
    car_rebuild();
}

void car_photo_reset(void)
{
    s_view = 0;
    free_full();
}
