#include "car.h"
#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static FILE *g_log_fp = NULL;
static char g_log_path[64] = "logs/carmode.log";

static void app_log_init(void)
{
    if (mkdir("logs", 0755) < 0 && errno != EEXIST)
        fprintf(stderr, "[log] mkdir logs: %s\n", strerror(errno));

    g_log_fp = fopen(g_log_path, "a");
    if (g_log_fp == NULL) {
        /* 运行目录不可写时退回 /tmp，保证还能抓到日志 */
        snprintf(g_log_path, sizeof(g_log_path), "/tmp/carmode.log");
        g_log_fp = fopen(g_log_path, "a");
    }

    log_set_level(LOG_DEBUG);
    log_set_quiet(false);

    if (g_log_fp != NULL) {
        if (log_add_fp(g_log_fp, LOG_DEBUG) != 0)
            fprintf(stderr, "[log] add file callback failed\n");
    } else {
        fprintf(stderr, "[log] open log file failed: %s\n", strerror(errno));
    }
}

static void app_log_close(void)
{
    if (g_log_fp != NULL) {
        fclose(g_log_fp);
        g_log_fp = NULL;
    }
}

int main(void)
{
    int ret;

    app_log_init();

    log_info("========================================");
    log_info(" GEC6818 智能车载中控");
    log_info(" 800x480 framebuffer + touch + bmp + ttf");
    log_info(" build: %s %s", __DATE__, __TIME__);
    log_info(" log file: %s", g_log_path);
    log_info(" 退出：Ctrl+C");
    log_info("========================================");

    ret = car_run();

    log_info("carmode exit: %d", ret);
    app_log_close();
    return ret;
}
