#ifndef __MP3_H__
#define __MP3_H__

/*
 * 车载音乐控制模块（对应参考工程的 mp3.c）
 *
 * 参考工程的 mp3.c 是一个“整屏阻塞式”的独立播放器：
 *   mp3_init() 内部 while(1) 自己读触摸、自己刷图，退出才返回。
 * 车机是多页面状态机，主循环不能被音乐页卡住，所以这里拆成：
 *   - mp3.c      ：只负责 madplay 子进程和播放状态
 *   - car_pages.c：音乐页的界面和触摸
 * 这样切到导航/仪表时音乐不会被阻塞。
 */

/* 播放第 index 首（会先停掉上一首） */
void car_music_start(int index);

/* 播放 / 暂停 切换 */
void car_music_toggle(void);

/* 停止并清空播放状态 */
void car_music_stop(void);

/* 下一首 / 上一首 */
void car_music_next(void);
void car_music_prev(void);

#endif
