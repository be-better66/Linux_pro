#include "car.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * 音频播放：调用 madplay。
 *
 * audio=1：真的调用 madplay 播放 res/music/ 下的 mp3
 * audio=0：纯模拟，不启动任何子进程，界面进度照常走
 *
 * 没有 madplay 或文件缺失时，system() 会失败，但播放状态仍然推进，
 * 所以音乐页不会卡死，只是没有声音。
 */

void car_music_stop(void)
{
    if (g_car.cfg.audio)
        system("killall -9 madplay 2>/dev/null");

    g_car.music_playing = 0;
    g_car.music_started = 0;
    g_car.music_pos = 0;
}

void car_music_start(int index)
{
    char cmd[600];

    if (g_car.cfg.song_count <= 0)
        return;

    if (index < 0)
        index = 0;
    if (index >= g_car.cfg.song_count)
        index = 0;

    car_music_stop();

    g_car.music_index = index;
    g_car.music_pos = 0;
    g_car.music_started = 1;
    g_car.music_playing = 1;
    g_car.last_music = car_now_ms();

    if (g_car.cfg.audio) {
        snprintf(cmd, sizeof(cmd),
                 "madplay -a 18 '%s' >/dev/null 2>&1 &",
                 g_car.cfg.music[index]);
        system(cmd);
        printf("[mp3] 播放: %s\n", g_car.cfg.music[index]);
    } else {
        printf("[mp3] 模拟播放: %s\n", g_car.cfg.music[index]);
    }

    if (g_car.page == CAR_PAGE_MUSIC)
        car_page_render();
}

void car_music_toggle(void)
{
    if (g_car.cfg.song_count <= 0)
        return;

    if (!g_car.music_started) {
        car_music_start(g_car.music_index);
        return;
    }

    if (g_car.music_playing) {
        if (g_car.cfg.audio)
            system("killall -SIGSTOP madplay 2>/dev/null");
        g_car.music_playing = 0;
    } else {
        if (g_car.cfg.audio)
            system("killall -SIGCONT madplay 2>/dev/null");
        g_car.music_playing = 1;
        g_car.last_music = car_now_ms();
    }
}

void car_music_next(void)
{
    int n = g_car.cfg.song_count;

    if (n <= 0)
        return;
    car_music_start((g_car.music_index + 1) % n);
}

void car_music_prev(void)
{
    int n = g_car.cfg.song_count;

    if (n <= 0)
        return;
    car_music_start((g_car.music_index + n - 1) % n);
}
