#include "car.h"

#include <stdio.h>

int main(void)
{
    printf("========================================\n");
    printf(" GEC6818 智能车载中控\n");
    printf(" 800x480 framebuffer + touch + bmp + ttf\n");
    printf(" 退出：Ctrl+C\n");
    printf("========================================\n");

    return car_run();
}
