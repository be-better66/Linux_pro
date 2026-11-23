#include "bmp.h"
#include "lcd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t le16(const unsigned char *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t le32(const unsigned char *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

int bmp_load(const char *path, bmp_image_t *img)
{
    FILE *fp;
    unsigned char hdr[54];
    uint32_t off, compression;
    int32_t width, height;
    uint16_t bpp;
    int topdown, abs_h, bytes, row;
    size_t row_bytes;
    unsigned char *line = NULL;
    uint32_t *pixels = NULL;

    if (path == NULL || img == NULL)
        return -1;
    memset(img, 0, sizeof(*img));

    fp = fopen(path, "rb");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    if (fread(hdr, 1, sizeof(hdr), fp) != sizeof(hdr)) {
        fprintf(stderr, "[bmp] %s: file too short\n", path);
        fclose(fp);
        return -1;
    }
    if (hdr[0] != 'B' || hdr[1] != 'M') {
        fprintf(stderr, "[bmp] %s: not a BMP file\n", path);
        fclose(fp);
        return -1;
    }

    off         = le32(hdr + 10);
    width       = (int32_t)le32(hdr + 18);
    height      = (int32_t)le32(hdr + 22);
    bpp         = le16(hdr + 28);
    compression = le32(hdr + 30);

    if (compression != 0) {
        fprintf(stderr, "[bmp] %s: compressed BMP not supported\n", path);
        fclose(fp);
        return -1;
    }
    if (width <= 0 || height == 0 || width > 4096 || height > 4096 || height < -4096) {
        fprintf(stderr, "[bmp] %s: bad size %dx%d\n", path, width, height);
        fclose(fp);
        return -1;
    }
    if (bpp != 24 && bpp != 32) {
        fprintf(stderr, "[bmp] %s: %d bpp not supported (need 24 or 32)\n", path, bpp);
        fclose(fp);
        return -1;
    }

    topdown = (height < 0);
    abs_h = topdown ? -height : height;
    bytes = bpp / 8;
    row_bytes = ((size_t)width * (size_t)bytes + 3u) & ~(size_t)3u;

    pixels = (uint32_t *)malloc((size_t)width * (size_t)abs_h * sizeof(uint32_t));
    line = (unsigned char *)malloc(row_bytes);
    if (pixels == NULL || line == NULL) {
        fprintf(stderr, "[bmp] %s: out of memory\n", path);
        free(pixels);
        free(line);
        fclose(fp);
        return -1;
    }

    if (fseek(fp, (long)off, SEEK_SET) != 0) {
        fprintf(stderr, "[bmp] %s: seek failed\n", path);
        free(pixels);
        free(line);
        fclose(fp);
        return -1;
    }

    for (row = 0; row < abs_h; row++) {
        int dst_y, x;
        uint32_t *dst;

        if (fread(line, 1, row_bytes, fp) != row_bytes) {
            fprintf(stderr, "[bmp] %s: truncated pixel data\n", path);
            free(pixels);
            free(line);
            fclose(fp);
            return -1;
        }

        dst_y = topdown ? row : (abs_h - 1 - row);
        dst = pixels + (size_t)dst_y * (size_t)width;

        for (x = 0; x < width; x++) {
            const unsigned char *s = line + (size_t)x * (size_t)bytes;
            dst[x] = ((uint32_t)s[2] << 16) | ((uint32_t)s[1] << 8) | (uint32_t)s[0];
        }
    }

    free(line);
    fclose(fp);

    img->width = width;
    img->height = abs_h;
    img->pixels = pixels;
    return 0;
}

void bmp_free(bmp_image_t *img)
{
    if (img == NULL)
        return;
    free(img->pixels);
    img->pixels = NULL;
    img->width = 0;
    img->height = 0;
}

int bmp_show(const char *path, int x, int y)
{
    bmp_image_t img;
    if (bmp_load(path, &img) < 0)
        return -1;
    lcd_blit(x, y, img.pixels, img.width, img.height);
    bmp_free(&img);
    return 0;
}

int bmp_load_fit(const char *path, int screen_w, int screen_h, uint32_t *dst)
{
    bmp_image_t img;
    int ox, oy, y, copy_w, copy_h;

    if (dst == NULL)
        return -1;

    if (bmp_load(path, &img) < 0) {
        memset(dst, 0, (size_t)screen_w * (size_t)screen_h * sizeof(uint32_t));
        return -1;
    }

    memset(dst, 0, (size_t)screen_w * (size_t)screen_h * sizeof(uint32_t));

    ox = (screen_w - img.width) / 2;
    oy = (screen_h - img.height) / 2;

    copy_w = img.width;
    copy_h = img.height;

    if (ox < 0) { copy_w += ox; ox = 0; }
    if (oy < 0) { copy_h += oy; oy = 0; }
    if (ox + copy_w > screen_w) copy_w = screen_w - ox;
    if (oy + copy_h > screen_h) copy_h = screen_h - oy;

    for (y = 0; y < copy_h; y++) {
        int src_y = (img.height - copy_h) / 2 + y;
        int src_x = (img.width - copy_w) / 2;
        memcpy(dst + (size_t)(oy + y) * (size_t)screen_w + (size_t)ox,
               img.pixels + (size_t)src_y * (size_t)img.width + (size_t)src_x,
               (size_t)copy_w * sizeof(uint32_t));
    }

    bmp_free(&img);
    return 0;
}
