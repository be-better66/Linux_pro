#include "touch.h"
#include "log.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

static int g_fd = -1;
static int g_min_x = 0, g_max_x = 1023;
static int g_min_y = 0, g_max_y = 599;

static int g_pressed = 0;
static int g_have_x = 0, g_have_y = 0;
static int g_raw_x = 0, g_raw_y = 0;
static int g_have_start = 0;
static int g_start_raw_x = 0, g_start_raw_y = 0;

#define TAP_SLOP 60

static int map_axis(int value, int min, int max, int size)
{
    if (max <= min)
        return value;
    if (value < min)
        value = min;
    if (value > max)
        value = max;
    return (value - min) * (size - 1) / (max - min);
}

static int screen_x(int raw) { return map_axis(raw, g_min_x, g_max_x, 800); }
static int screen_y(int raw) { return map_axis(raw, g_min_y, g_max_y, 480); }

int touch_init(void)
{
    struct input_absinfo abs;

    if (g_fd >= 0)
        return 0;

    g_fd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
    if (g_fd < 0) {
        log_error("[touch] open /dev/input/event0: %s", strerror(errno));
        return -1;
    }

    memset(&abs, 0, sizeof(abs));
    if (ioctl(g_fd, EVIOCGABS(ABS_X), &abs) == 0 && abs.maximum > abs.minimum) {
        g_min_x = abs.minimum;
        g_max_x = abs.maximum;
    }
    memset(&abs, 0, sizeof(abs));
    if (ioctl(g_fd, EVIOCGABS(ABS_Y), &abs) == 0 && abs.maximum > abs.minimum) {
        g_min_y = abs.minimum;
        g_max_y = abs.maximum;
    }

    log_info("[touch] raw x:%d..%d  y:%d..%d -> 800x480",
             g_min_x, g_max_x, g_min_y, g_max_y);
    return 0;
}

void touch_close(void)
{
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}

static void remember_start(void)
{
    if (g_pressed && !g_have_start && g_have_x && g_have_y) {
        g_start_raw_x = g_raw_x;
        g_start_raw_y = g_raw_y;
        g_have_start = 1;
    }
}

static touch_event_type_t classify(touch_event_t *ev)
{
    int ex = screen_x(g_raw_x);
    int ey = screen_y(g_raw_y);
    int sx = g_have_start ? screen_x(g_start_raw_x) : ex;
    int sy = g_have_start ? screen_y(g_start_raw_y) : ey;
    int dx = ex - sx;
    int dy = ey - sy;

    ev->x = ex;
    ev->y = ey;
    ev->start_x = sx;
    ev->start_y = sy;

    if (abs(dx) < TAP_SLOP && abs(dy) < TAP_SLOP)
        return TOUCH_EVT_TAP;
    if (abs(dx) >= abs(dy))
        return (dx > 0) ? TOUCH_EVT_SWIPE_RIGHT : TOUCH_EVT_SWIPE_LEFT;
    return (dy > 0) ? TOUCH_EVT_SWIPE_DOWN : TOUCH_EVT_SWIPE_UP;
}

static int handle_event(const struct input_event *ie, touch_event_t *ev)
{
    if (ie->type == EV_ABS) {
        if (ie->code == ABS_X || ie->code == ABS_MT_POSITION_X) {
            g_raw_x = ie->value;
            g_have_x = 1;
            remember_start();
        } else if (ie->code == ABS_Y || ie->code == ABS_MT_POSITION_Y) {
            g_raw_y = ie->value;
            g_have_y = 1;
            remember_start();
        }
    } else if (ie->type == EV_KEY && ie->code == BTN_TOUCH) {
        if (ie->value == 1) {
            g_pressed = 1;
            g_have_start = 0;
            /*
             * 有些 GEC6818 驱动先报 ABS_X/ABS_Y，再报 BTN_TOUCH=1。
             * 这里不能再清坐标，否则纯点击没有起点，只有滑动才会产生新坐标。
             */
            remember_start();
        } else if (ie->value == 0) {
            if (g_pressed && g_have_x && g_have_y) {
                ev->type = classify(ev);
                g_pressed = 0;
                g_have_start = 0;
                g_have_x = 0;
                g_have_y = 0;
                return 1;
            }
            g_pressed = 0;
            g_have_start = 0;
            g_have_x = 0;
            g_have_y = 0;
        }
    }
    return 0;
}

int touch_wait(touch_event_t *ev, int timeout_ms)
{
    fd_set rfds;
    struct timeval tv;
    struct timeval *ptv = NULL;
    int ret;

    if (ev == NULL)
        return -1;
    if (g_fd < 0)
        return -1;

    FD_ZERO(&rfds);
    FD_SET(g_fd, &rfds);

    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        ptv = &tv;
    }

    ret = select(g_fd + 1, &rfds, NULL, NULL, ptv);
    if (ret == 0)
        return 0;
    if (ret < 0) {
        if (errno == EINTR)
            return 0;
        log_error("[touch] select: %s", strerror(errno));
        return -1;
    }

    for (;;) {
        struct input_event ie;
        ssize_t n = read(g_fd, &ie, sizeof(ie));

        if (n == (ssize_t)sizeof(ie)) {
            if (handle_event(&ie, ev))
                return 1;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return 0;
        } else if (n == 0) {
            return 0;
        } else if (n < 0) {
            if (errno == EINTR)
                continue;
            log_error("[touch] read: %s", strerror(errno));
            return -1;
        } else {
            return 0;
        }
    }
}
