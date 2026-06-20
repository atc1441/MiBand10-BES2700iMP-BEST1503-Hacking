#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

static int   fb_width = 1920, fb_height = 1080;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

/* Convert an ARGB8888 image to gray and indexed 8. Since it's gray, clut is Linearly gray. */

static void convert_to_index_gray8(vg_lite_buffer_t *src, vg_lite_buffer_t *dst)
{
    uint32_t r, g, b;
    uint32_t x, y;
    uint32_t *src_pixel;
    uint8_t  *dst_pixel;

    /* Create dst buffer. */
    dst->width = src->width;
    dst->height = dst->height;
    dst->format = VG_LITE_INDEX_8;
    dst->stride = 0;
    vg_lite_allocate(dst);

    src_pixel = (uint32_t *)src->memory;
    dst_pixel = (uint8_t * )dst->memory;
    for (y = 0; y < src->height; y++) {
        for (x = 0; x < src->width; x++) {
            r = (src_pixel[x] >> 16) & 0xff;
            g = (src_pixel[x] >> 8 ) & 0xff;
            b = src_pixel[x] & 0xff;

            dst_pixel[x] = (r + g + b) / 3;
        }
        src_pixel += src->width;
        dst_pixel += dst->width;
    }
}

static int32_t update_clut(uint32_t *clut, uint32_t *size, uint32_t value, uint32_t limit)
{
    int32_t i = 0;

    for (i = 0; i < *size; i++) {
        if (value == clut[i]) {
            break;
        }
    }

    if (i >= limit) {
        return -1;
    }

    if (i == *size) {
        clut[*size] = value;
        (*size)++;
    }
    return i;
}

/* Convert an ARGB8888 image to and indexed image. */
static int32_t convert_to_index(int32_t index, vg_lite_buffer_t *src, vg_lite_buffer_t *dst, uint32_t *clut)
{
    uint32_t mask = 0xffffffff, limit;
    uint32_t clut_size = 0;
    uint32_t x, y;
    int32_t i, j, left;
    uint32_t *src_pixel, src_value;
    uint8_t  *dst_pixels, *dst_orig;
    uint8_t  *index_cache, *ic_orig, *src_orig;

    switch (index) {
        case 1:
            mask = 0x80808080;
            limit = 2;
            break;

        case 2:
            mask = 0xc0c0c0c0;
            limit = 4;
            break;

        case 4:
            mask = 0xf0f0f0f0;
            limit = 16;
            break;

        case 8:
            mask = 0xffffffff;
            limit = 256;
            break;

        default:
            return -1;
            break;
    }

    mask = ~0;
    src_pixel = (uint32_t *) src->memory;
    src_orig = (uint8_t *)src_pixel;
    index_cache = (uint8_t *) malloc(src->width * src->height);
    ic_orig = (uint8_t *)index_cache;

    /* Update CLUT and Fill in Index Cache. */
    i = 0;
    for (y = 0; y < src->height; y++) {
        for (x = 0; x < src->width; x++) {
            src_value = src_pixel[x] & mask;
            index_cache[i] = update_clut(clut, &clut_size, src_value, limit);
            i++;
        }

        src_pixel = (uint32_t *) (src_orig + src->stride * y);
    }

    /* CLUT ready, index ready, now update dst pixels. */
    dst_pixels = (uint8_t *)dst->memory;
    dst_orig = dst_pixels;
    j = 0;
    for (y = 0; y < dst->height; y++) {
        dst_pixels = dst_orig + dst->stride * y;
        for (x = 0; x < dst->width; x += 8 / index) {
            src_value = 0;

            if (dst->width - x < 8 / index) {
                left = dst->width - x;
            } else {
                left = 8/index;
            }

            for (i = 0; i < left * index; i += index) {
                src_value |= (index_cache[j++] << i);
            }

            dst_pixels[x * index / 8] = src_value;
        }
    }
    return 0;
}

/* Image file format is: width + height + stride + format + pixels. */
static void save_img(vg_lite_buffer_t *buf, const char *name, uint32_t clut[])
{
    FILE *fp = fopen(name, "wb+");

    if (fp) {
#if 1
        fwrite((void *)&buf->width, sizeof(buf->width), 1, fp);
        fwrite((void *)&buf->height, sizeof(buf->height), 1, fp);
        fwrite((void *)&buf->stride, sizeof(buf->stride), 1, fp);
        fwrite((void *)&buf->format, sizeof(buf->format), 1, fp);
        fwrite((void *)buf->memory, buf->stride * buf->height, 1, fp);

        switch (buf->format) {
            case VG_LITE_INDEX_1:
                fwrite(clut, sizeof(clut[0]) * 2, 1, fp);
                break;

            case VG_LITE_INDEX_2:
                fwrite(clut, sizeof(clut[0]) * 4, 1, fp);
                break;

            case VG_LITE_INDEX_4:
                fwrite(clut, sizeof(clut[0]) * 16, 1, fp);
                break;

            case VG_LITE_INDEX_8:
                fwrite(clut, sizeof(clut[0]) * 256, 1, fp);
                break;

            default:
                break;
        }
#else
        int clut_count, col;
        uint8_t *pixels = (uint8_t *)buf->memory;

        fprintf(fp, "int32_t img_width = %d;\n", buf->width);
        fprintf(fp, "int32_t img_height = %d;\n", buf->height);
        fprintf(fp, "int32_t img_stride = %d;\n", buf->stride);
        fprintf(fp, "int32_t img_format = %d;\n", buf->format);
        fprintf(fp, "uint8_t img_data[] = {\n");
        for (col = 0; col < buf->stride * buf->height; col++) {
            fprintf(fp, "0x%02x, ", *pixels++);
            if (col % 16 == 0) {
                fprintf(fp, "\n");
            }
        }
        fprintf(fp, "};\n");

        switch (buf->format) {
            case VG_LITE_INDEX_1:
                clut_count = 2;
                break;

            case VG_LITE_INDEX_2:
                clut_count = 4;
                break;

            case VG_LITE_INDEX_4:
                clut_count = 16;
                break;

            case VG_LITE_INDEX_8:
                clut_count = 256;
                break;

            default:
                clut_count = 0;
                break;
        }
        fprintf(fp, "uint32_t clut[] = {\n");
        for (col = 0; col < clut_count; col++) {
            fprintf(fp, "0x%08x, ", clut[col]);
            if (col % 8 == 0) {
                fprintf(fp, "\n");
            }
        }
        fprintf(fp, "};\n");
#endif
        fclose(fp);
    }
}

int main(int argc, const char * argv[])
{
    uint32_t clut[256];
    int i;
    for (i = 0; i < 256; i++)
    {
        clut[i] = 0xff000000 |
        (i << 16) | (i << 8) | (i);
    }
    /* Initialize the blitter. */
    vg_lite_init(fb_width, fb_height);

    vg_lite_buffer_t img;
    vg_lite_buffer_t img1;

    vg_lite_load_png(&img, "607/car_index8.png");

    img1.width = img.width;
    img1.height = img.height;
    img1.stride = 0;
    img1.format = VG_LITE_INDEX_8;
    vg_lite_allocate(&img1);
    /*convert_to_index_gray8(&img, &img1); */
    convert_to_index(8, &img, &img1, clut);
    save_img(&img1, "607/car8.img", clut);

    vg_lite_free(&img);
    vg_lite_free(&img1);
    /* Cleanup. */
    cleanup();
    return 0;
}
