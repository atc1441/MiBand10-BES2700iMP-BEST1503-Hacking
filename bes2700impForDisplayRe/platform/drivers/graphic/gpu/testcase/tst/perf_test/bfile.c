
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite.h"
#include "hal_trace.h"

#if 0
static int read_int(FILE *fp)
{
    unsigned char b0, b1; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);

    return ((int)b0 << 8 | b1);
}

// Read a 32-bit signed integer.
static int read_long(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

// Write a 32-bit signed integer.
static int write_long(FILE *fp,int  l)
{
    putc(l, fp);
    putc(l >> 8, fp);
    putc(l >> 16, fp);
    return (putc(l >> 24, fp));
}
#endif




int vg_lite_bf_load_pkm_info_to_buffer(vg_lite_buffer_t * buffer, const char * fdata)
{

    int status = 1;
    unsigned char * p = fdata;
    unsigned char b0 = 0;
    unsigned char b1 = 0;
    if(buffer == NULL)
        return -1;

    if (fdata != NULL) {
        //read_long(fp);
        //read_long(fp);
        p += 8;
        b0 = *(p);
        b1 = *(p + 1);
        buffer->width = ((int)b0 << 8 | b1);
        p += 2;
        b0 = *(p);
        b1 = *(p + 1);
        buffer->height = ((int)b0 << 8 | b1);
        buffer->format = VG_LITE_RGBA8888_ETC2_EAC;
        buffer->tiled = VG_LITE_TILED;
        status = 0;
    }
    return status;
}

int vg_lite_bf_load_pkm(vg_lite_buffer_t * buffer, const char * fdata)
{

    int status = 1;
    unsigned char * p = fdata;
    if(buffer == NULL)
        return -1;

    if (fdata != NULL) {
        int flag;

        //fseek(fp, 16, SEEK_SET);
        p += 16;
        memcpy(buffer->memory, p ,buffer->stride * buffer->height);
       // flag = fread(buffer->memory, buffer->stride * buffer->height, 1, fp);

        status = 0;
    }

    return status;
}

int vg_lite_bf_load_raw(vg_lite_buffer_t * buffer, const char * fdata)
{

    uint32_t  * p = fdata;
    // Set status.
    int status = 1;
    // Check the result with golden.
    if (fdata != NULL) {
        int flag;

        // Get width, height, stride and format info.

        buffer->width  = *p;
        buffer->height = *(p +1);
        buffer->stride = *(p + 2);
        buffer->format = *(p + 3);
        // Allocate the VGLite buffer memory.
        if (vg_lite_allocate(buffer) != VG_LITE_SUCCESS)
        {
            return -1;
        }

        //fseek(fp, 16, SEEK_SET);
        p += 4;
        //flag = fread(buffer->memory, buffer->stride * buffer->height, 1, fp);
        memset(buffer->memory, p ,buffer->stride * buffer->height);
        status = 0;
    }

    // Return the status.
    return status;
}

int vg_lite_bf_load_raw_byline(vg_lite_buffer_t * buffer, const char * fdata)
{
    uint32_t  * p = fdata;

    // Set status.
    int status = 1;

    // Check the result with golden.
    if (fdata != NULL) {
        int flag;

        // Get width, height, stride and format info.
        buffer->width  = *(p);//read_long(fp);
        buffer->height = *(p + 1);//read_long(fp);
        buffer->stride = *(p + 2);//read_long(fp);
        buffer->format = *(p + 3);//read_long(fp);

        // Allocate the VGLite buffer memory.
        if (vg_lite_allocate(buffer) != VG_LITE_SUCCESS)
        {
            return -1;
        }

        //fseek(fp, 16, SEEK_SET);
        p += 4;
        memcpy(buffer->memory, p, buffer->width * buffer->height * (buffer->stride / buffer->width));
        //flag = fread(buffer->memory, buffer->width * buffer->height * (buffer->stride / buffer->width), 1, fp);

        status = 0;
    }

    // Return the status.
    return status;
}


