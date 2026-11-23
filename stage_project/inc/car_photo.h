#ifndef __CAR_PHOTO_H__
#define __CAR_PHOTO_H__

#include "touch.h"

/* 相册页：缩略图 + 全屏查看 */
void car_photo_enter(void);
void car_photo_render(void);
void car_photo_touch(const touch_event_t *ev);

/* car.c 用：判断是否在全屏查看、返回缩略图、离开页面时复位 */
int  car_photo_view_active(void);
void car_photo_back_to_grid(void);
void car_photo_reset(void);

#endif
