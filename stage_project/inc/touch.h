#ifndef __TOUCH_H__
#define __TOUCH_H__

/*
 * GEC6818 触摸屏驱动（/dev/input/event0）
 * 同时支持点击和上下左右滑动
 */

typedef enum {
    TOUCH_EVT_TAP = 1,       /* 点击 */
    TOUCH_EVT_SWIPE_LEFT,    /* 左滑 */
    TOUCH_EVT_SWIPE_RIGHT,   /* 右滑 */
    TOUCH_EVT_SWIPE_UP,      /* 上滑 */
    TOUCH_EVT_SWIPE_DOWN     /* 下滑 */
} touch_event_type_t;

typedef struct {
    touch_event_type_t type;
    int x, y;                /* 抬手位置（屏幕坐标） */
    int start_x, start_y;    /* 按下位置（屏幕坐标） */
} touch_event_t;

/* 成功返回 0，失败返回 -1 */
int  touch_init(void);
void touch_close(void);

/*
 * 等待一次完整的触摸事件。
 * timeout_ms < 0 表示一直等待；=0 表示只查询一次不阻塞。
 * 返回 1：读到事件；0：超时；-1：设备错误
 */
int touch_wait(touch_event_t *ev, int timeout_ms);

#endif
