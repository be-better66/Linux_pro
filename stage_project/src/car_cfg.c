#include "car.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static void trim(char *s)
{
    char *p = s;
    size_t len;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);

    len = strlen(s);
    while (len > 0) {
        char c = s[len - 1];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            s[--len] = 0;
        else
            break;
    }
}

static void copy_str(char *dst, size_t size, const char *src)
{
    if (size == 0)
        return;
    snprintf(dst, size, "%s", src ? src : "");
}

static void add_song(car_config_t *cfg, const char *value_in)
{
    char buf[1024];
    char *title, *artist, *path, *len;
    int idx;

    if (cfg->song_count >= CAR_MAX_SONGS || value_in == NULL)
        return;

    /* 默认歌曲是字符串常量，不能直接改写，先复制到可写缓冲区 */
    snprintf(buf, sizeof(buf), "%s", value_in);

    title = buf;
    artist = strchr(title, '|');
    if (artist == NULL)
        return;
    *artist++ = 0;

    path = strchr(artist, '|');
    if (path == NULL)
        return;
    *path++ = 0;

    len = strchr(path, '|');
    if (len != NULL)
        *len++ = 0;

    trim(title);
    trim(artist);
    trim(path);

    idx = cfg->song_count;
    copy_str(cfg->song_title[idx], CAR_NAME_LEN, title);
    copy_str(cfg->song_artist[idx], CAR_NAME_LEN, artist);
    copy_str(cfg->music[idx], CAR_PATH_LEN, path);
    cfg->song_length[idx] = (len != NULL) ? atoi(len) : 240;
    if (cfg->song_length[idx] <= 0)
        cfg->song_length[idx] = 240;
    cfg->song_count++;
}

static void add_default_songs(car_config_t *cfg)
{
    add_song(cfg, "夜空中最亮的星|逃跑计划|res/music/1.mp3|3");
    add_song(cfg, "平凡之路|朴树|res/music/2.mp3|3");
    add_song(cfg, "海阔天空|Beyond|res/music/3.mp3|3");
}

void car_config_defaults(car_config_t *cfg)
{
    if (cfg == NULL)
        return;
    memset(cfg, 0, sizeof(*cfg));

    cfg->night = 0;
    cfg->volume = 12;
    cfg->temperature = 24;
    cfg->demo = 1;
    cfg->audio = 1;
    cfg->page_anim = 0;

    copy_str(cfg->font, CAR_PATH_LEN, "res/font/simfang.ttf");
    copy_str(cfg->logo, CAR_PATH_LEN, "res/car/logo.bmp");
    copy_str(cfg->bg_day, CAR_PATH_LEN, "res/car/bg_day.bmp");
    copy_str(cfg->bg_night, CAR_PATH_LEN, "res/car/bg_night.bmp");
    copy_str(cfg->icon_nav, CAR_PATH_LEN, "res/car/icon_nav.bmp");
    copy_str(cfg->icon_dash, CAR_PATH_LEN, "res/car/icon_dash.bmp");
    copy_str(cfg->icon_music, CAR_PATH_LEN, "res/car/icon_music.bmp");
    copy_str(cfg->icon_phone, CAR_PATH_LEN, "res/car/icon_phone.bmp");
    copy_str(cfg->icon_setting, CAR_PATH_LEN, "res/car/icon_setting.bmp");
    copy_str(cfg->icon_pic, CAR_PATH_LEN, "res/car/icon_pic.bmp");
    copy_str(cfg->icon_sun, CAR_PATH_LEN, "res/car/icon_sun.bmp");
    copy_str(cfg->icon_moon, CAR_PATH_LEN, "res/car/icon_moon.bmp");
    copy_str(cfg->dash_bg, CAR_PATH_LEN, "res/car/dash_bg.bmp");
    copy_str(cfg->album, CAR_PATH_LEN, "res/car/album.bmp");
    copy_str(cfg->photo_dir, CAR_PATH_LEN, "res/photo");
    copy_str(cfg->reverse, CAR_PATH_LEN, "res/car/reverse.bmp");

    cfg->song_count = 0;
    add_default_songs(cfg);
}

int car_config_load(const char *path, car_config_t *cfg)
{
    FILE *fp;
    char line[1024];

    if (path == NULL || cfg == NULL)
        return -1;

    car_config_defaults(cfg);

    fp = fopen(path, "r");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    /* 配置文件里的 song= 会追加到默认歌曲后，先清空 */
    cfg->song_count = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *eq, *key, *value;

        trim(line);
        if (line[0] == 0 || line[0] == '#' || line[0] == ';')
            continue;

        eq = strchr(line, '=');
        if (eq == NULL)
            continue;
        *eq = 0;
        key = line;
        value = eq + 1;
        trim(key);
        trim(value);

        if (strcasecmp(key, "night") == 0) {
            cfg->night = atoi(value) ? 1 : 0;
        } else if (strcasecmp(key, "volume") == 0) {
            cfg->volume = atoi(value);
            if (cfg->volume < 0) cfg->volume = 0;
            if (cfg->volume > 30) cfg->volume = 30;
        } else if (strcasecmp(key, "temperature") == 0) {
            cfg->temperature = atoi(value);
            if (cfg->temperature < 16) cfg->temperature = 16;
            if (cfg->temperature > 30) cfg->temperature = 30;
        } else if (strcasecmp(key, "demo") == 0) {
            cfg->demo = atoi(value) ? 1 : 0;
        } else if (strcasecmp(key, "audio") == 0) {
            cfg->audio = atoi(value) ? 1 : 0;
        } else if (strcasecmp(key, "page_anim") == 0) {
            cfg->page_anim = atoi(value) ? 1 : 0;
        } else if (strcasecmp(key, "font") == 0) {
            copy_str(cfg->font, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "logo") == 0) {
            copy_str(cfg->logo, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "bg_day") == 0) {
            copy_str(cfg->bg_day, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "bg_night") == 0) {
            copy_str(cfg->bg_night, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_nav") == 0) {
            copy_str(cfg->icon_nav, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_dash") == 0) {
            copy_str(cfg->icon_dash, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_music") == 0) {
            copy_str(cfg->icon_music, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_phone") == 0) {
            copy_str(cfg->icon_phone, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_setting") == 0) {
            copy_str(cfg->icon_setting, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_pic") == 0) {
            copy_str(cfg->icon_pic, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_sun") == 0) {
            copy_str(cfg->icon_sun, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "icon_moon") == 0) {
            copy_str(cfg->icon_moon, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "dash_bg") == 0) {
            copy_str(cfg->dash_bg, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "album") == 0) {
            copy_str(cfg->album, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "photo_dir") == 0) {
            copy_str(cfg->photo_dir, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "reverse") == 0) {
            copy_str(cfg->reverse, CAR_PATH_LEN, value);
        } else if (strcasecmp(key, "song") == 0) {
            add_song(cfg, value);
        }
    }

    fclose(fp);

    if (cfg->song_count == 0) {
        fprintf(stderr, "[car] no song in config, use default songs\n");
        add_default_songs(cfg);
    }

    return 0;
}
