//-----------------------------------------------------------------------------
//Description: The test cases test the blitting of different size/formats/transformations of image.
//-----------------------------------------------------------------------------
#include "SFT.h"
#include "golden.h"

#define     NUM_SRC_FORMATS 2
#define     NUM_DST_FORMATS 1

extern vg_lite_buffer_t raw;
extern vg_lite_filter_t filter;

extern int fb_width;
extern int fb_height;
/* Format array. */
static vg_lite_buffer_format_t formats[] =
{
    VG_LITE_RGBA8888,
    VG_LITE_RGB565,
};

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

/* Init buffers. */
vg_lite_error_t gen_buffer(int type, vg_lite_buffer_t *buf, vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    void *data = NULL;
    uint8_t *pdata8, *pdest8;
    uint32_t bpp = get_bpp(format);
    int i;

    //Randomly generate some images.
    data = gen_image(type, format, width, height);
    pdata8 = (uint8_t*)data;
    pdest8 = (uint8_t*)buf->memory;
    if (data != NULL)
    {
        for (i = 0; i < height; i++)
        {
            memcpy (pdest8, pdata8, bpp / 8 * width);
            pdest8 += buf->stride;
            pdata8 += bpp / 8 * width;
        }

        free(data);
    }
    else
    {
        printf("Image data generating failed.\n");
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height)
{
    vg_lite_error_t error;

    buffer->width = width;
    buffer->height = height;
    buffer->format = format;
    buffer->stride = 0;
    buffer->handle = NULL;
    buffer->memory = NULL;
    buffer->address = 0;
    buffer->tiled   = 0;

    error = vg_lite_allocate(buffer);

    return error;
}

static void Free_Buffer(vg_lite_buffer_t *buffer)
{
    vg_lite_free(buffer);
}

/*
 Test 1: Blit for same size between different src formats.
 then blit src to same dest formats.
 */
void     Blit_001()
{
    FILE * fp;
    char casename[30];
    int ret = 0;
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    int32_t    width, height;
    char * golden[] = {golden1, golden2};
    int loop;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, fb_width);
        height = (int32_t)Random_r(1.0f, fb_height);
        
        error = Allocate_Buffer(&src_buf, formats[i], width, height);
        if (error != VG_LITE_SUCCESS)
        {
            printf("[%s: %d]allocate srcbuff %d failed.\n", __func__, __LINE__, i);
            return;
        }
        error = gen_buffer(i % 2, &src_buf, formats[i], src_buf.width, src_buf.height);
        src_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
        
        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            error = Allocate_Buffer(&dst_buf, formats[1], width, height);
            if (error != VG_LITE_SUCCESS)
            {
                printf("[%s: %d]allocate dstbuff %d failed.\n", __func__, __LINE__, j);
                Free_Buffer(&src_buf);
                return;
            }
            
            vg_lite_clear(&dst_buf, NULL, cc);
            
            printf("\nblit with src format: %d\n", formats[i]);
            error = vg_lite_blit(&dst_buf, &src_buf, NULL, 0, 0, filter);
            vg_lite_finish();
            
            SaveBMP_SFT("Blit_001_",&dst_buf);
            
            // Check the result with golden.
            fp = fopen(golden[i], "rb");
            if (fp != NULL) {
                int flag;
                
                // Get width, height, stride and format info.
                (&raw)->width  = read_long(fp);
                (&raw)->height = read_long(fp);
                (&raw)->stride = read_long(fp);
                (&raw)->format = read_long(fp);
                
                // Allocate the VGLite buffer memory.
                if (vg_lite_allocate(&raw) != 0)
                    printf("allocate error!\n");
                
                fseek(fp, 16, SEEK_SET);
                flag = fread((&raw)->memory, (&raw)->width * (&raw)->height * 2, 1, fp);
                if(flag != 1) {
                    printf("failed to read raw buffer data\n");
                }
                
                for (loop = 0; loop < (&dst_buf)->height; loop++) {
                    if (memcmp((&raw)->memory, (&dst_buf)->memory, (&dst_buf)->width * 2)) {
                        ret = 1;
                        break;
                    }
                }
                if (ret) {
                    printf("\ngfx_4_%d result error\n", i + 1);
                } else {
                    printf("\ngfx_4_%d result correct\n", i + 1);
                }
                
                fclose(fp);
                fp = NULL;
            }
            if (raw.handle != NULL) {
                // Free the raw memory.
                vg_lite_free(&raw);
            }
            
            // Save RAW file.
            if (getenv("GOLDEN") != NULL) {
                if (!strcmp(getenv("GOLDEN"), "1")) {
                    unsigned char* pt;
                    int mem;
                    sprintf(casename, "gfx_4_%d.raw", i + 1);
                    fp = fopen(casename, "w");
                    
                    if (fp == NULL) {
                        printf("error!\n");
                    }
                    
                    // Save width, height, stride and format info.
                    write_long(fp, (&dst_buf)->width);
                    write_long(fp, (&dst_buf)->height);
                    write_long(fp, (&dst_buf)->stride);
                    write_long(fp, (&dst_buf)->format);
                    
                    // Save buffer info.
                    pt = (unsigned char*) (&dst_buf)->memory;
                    for (loop = 0; loop < (&dst_buf)->height; loop++) {
                        for (mem = 0; mem < (&dst_buf)->width * 2; mem++) {
                            putc(pt[mem], fp);
                        }
                    }
                    
                    fclose(fp);
                    fp = NULL;
                }
            }

            //Free the dest buff.
            Free_Buffer(&dst_buf);
        }
        
        //Free the src buff.
        Free_Buffer(&src_buf);
    }
}

// Same format, different size, width scale matrix, no blending.
void     Blit_002()
{
    FILE * fp;
    char casename[30];
    int ret = 0;
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    vg_lite_matrix_t matrix;
    int32_t width, height;
    int loop;
    char * golden[] = {golden3, golden4};

    width = (int32_t)Random_r(1.0f, fb_width);
    height = (int32_t)Random_r(1.0f, fb_height);

    error = Allocate_Buffer(&src_buf, formats[0], width, height);
    if (error != VG_LITE_SUCCESS)
    {
        printf("[%s: %d]allocate srcbuffer failed.", __func__, __LINE__);
        return;
    }
    //Regenerate src buffers. Check is good for transformation.
    vg_lite_clear(&src_buf, NULL, cc);
    vg_lite_finish();
    error = gen_buffer(0, &src_buf, formats[0], src_buf.width, src_buf.height);
    if (error != VG_LITE_SUCCESS)
    {
        printf("[%s: %d]generate srcbuffer failed.", __func__, __LINE__);
        Free_Buffer(&src_buf);
        return;
    }
    src_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;

    //Then blit src to different size dests.
    for (i = 0; i < 2; i++)
    {
        width = (int32_t)Random_r(1.0f, fb_width);
        height = (int32_t)Random_r(1.0f, fb_height);

        error = Allocate_Buffer(&dst_buf, formats[1], width, height);
        if (error != VG_LITE_SUCCESS)
        {
            printf("[%s: %d]allocate dstbuffer %d failed.", __func__, __LINE__, i);
            return;
        }

        vg_lite_identity(&matrix);
        
        printf("\nblit buffer size: %d x %d\n", (&dst_buf)->width, (&dst_buf)->height);
        vg_lite_clear(&dst_buf, NULL, cc);
        error = vg_lite_blit(&dst_buf, &src_buf,NULL /*&matrix*/, 0, 0, filter);
        vg_lite_finish();

        SaveBMP_SFT("Blit_002_",&dst_buf);
        
        // Save RAW file.
        if (getenv("GOLDEN") != NULL) {
            if (!strcmp(getenv("GOLDEN"), "1")) {
                unsigned char* pt ;
                int mem;
                sprintf(casename, "gfx_9_10_%d.raw", i + 1);
                fp = fopen(casename, "w");
                
                if (fp == NULL) {
                    printf("error!\n");
                }
                
                // Save width, height, stride and format info.
                write_long(fp, (&dst_buf)->width);
                write_long(fp, (&dst_buf)->height);
                write_long(fp, (&dst_buf)->stride);
                write_long(fp, (&dst_buf)->format);
                
                // Save buffer info.
                pt = (unsigned char*) (&dst_buf)->memory;
                for (loop = 0; loop < (&dst_buf)->height; loop++) {
                    for (mem = 0; mem < (&dst_buf)->width * 2; mem++) {
                        putc(pt[mem], fp);
                    }
                }
                
                fclose(fp);
                fp = NULL;
            }
        }
        
        // Check the result with golden.
        fp = fopen(golden[i], "rb");
        if (fp != NULL) {
            int flag;
            
            // Get width, height, stride and format info.
            (&raw)->width  = read_long(fp);
            (&raw)->height = read_long(fp);
            (&raw)->stride = read_long(fp);
            (&raw)->format = read_long(fp);
            
            // Allocate the VGLite buffer memory.
            if (vg_lite_allocate(&raw) != 0)
                printf("allocate error!\n");
            
            fseek(fp, 16, SEEK_SET);
            flag = fread((&raw)->memory, (&raw)->width * (&raw)->height * 2, 1, fp);
            if(flag != 1) {
                printf("failed to read raw buffer data\n");
            }
            
            for (loop = 0; loop < (&dst_buf)->height; loop++) {
                if (memcmp((&raw)->memory, (&dst_buf)->memory, (&dst_buf)->width * 2)) {
                    ret = 1;
                    break;
                }
            }
            if (ret) {
                printf("\ngfx_9_10_%d result error\n", i + 1);
            } else {
                printf("\ngfx_9_10_%d result correct\n", i + 1);
            }
            
            fclose(fp);
            fp = NULL;
        }
        if (raw.handle != NULL) {
            // Free the raw memory.
            vg_lite_free(&raw);
        }

        //Free dest buff.
        Free_Buffer(&dst_buf);
    }

    //Free src buff.
    Free_Buffer(&src_buf);
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
void SFT_Blit()
{
    printf("\nCase: Blit:::::::::::::::::::::Started\n");
    
    printf("\nCase: Blit_001:::::::::::::::::::::Started\n");
    Blit_001();
    printf("\nCase: Blit_001:::::::::::::::::::::Ended\n");
    
    printf("\nCase: Blit_002:::::::::::::::::::::Started\n");
    Blit_002();
    printf("\nCase: Blit_002:::::::::::::::::::::Ended\n");
    
    printf("\nCase: Blit:::::::::::::::::::::Ended\n");
}

