//-----------------------------------------------------------------------------
//Description: The test cases test the blitting of different size/formats/strides of image.
//-----------------------------------------------------------------------------
#include "SFT.h"

#define     NUM_SRC_FORMATS 2
#define     NUM_DST_FORMATS 1

static vg_lite_buffer_t * raw = NULL;
static int blt_width = 0;
static int blt_height = 0;
static vg_lite_filter_t blt_filter = VG_LITE_FILTER_POINT;

/* Format array. */
static vg_lite_buffer_format_t formats[] =
{
    VG_LITE_RGBA8888,
    VG_LITE_RGB565,
};

static void blt_cleanup(void)
{
    if (raw->handle != NULL) {
        // Free the raw memory.
        vg_lite_free(raw);
    }
}

/* Init buffers. */
vg_lite_error_t gen_buffer(int type, vg_lite_buffer_t *buf, vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    void *data = NULL;
    uint8_t *pdata8, *pdest8;
    uint32_t bpp = get_bpp(format);
    uint32_t i;

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

        FREE(data);
    }
    else
    {
        VGLITE_TST_PRINTF("Image data generating failed.\r\n");
    }

    return VG_LITE_SUCCESS;
}

static vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height, int32_t stride)
{
    vg_lite_error_t error;

    buffer->width = width;
    buffer->height = height;
    buffer->format = format;
    buffer->stride = stride;
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
 Resolution: Random generated
 Format: VG_LITE_RGBA8888
 Transformation: Translate/Scale
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_blit
 Description: Generate different random formats(VG_LITE_RGBA8888/VG_LITE_RGB565) source buffers, then blit them to same dest buffers.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
int     SFT_Blit_001()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    int ret = TB_OK;
    float x, y;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    vg_lite_matrix_t matrix;
    vg_lite_float_t xScl, yScl;
    int32_t width, height;

    for (i = 0; (i < NUM_SRC_FORMATS) && (TB_OK == ret); i++)
    {
        width = (int32_t)Random_r(1.0f, blt_width);
        height = (int32_t)Random_r(1.0f, blt_height);

        // Generate different source buffer formats.
        error = Allocate_Buffer(&src_buf, formats[i], width, height, 0);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s: %d]allocate srcbuffer %d failed.", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            return TB_BAD_API_FAIL;
        }
        //Regenerate src buffers. Check is good for transformation.
        error = gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s: %d]generate srcbuffer %d failed.", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            return TB_BAD_GENERIC;
        }
        src_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;

        //Then blit src to dest (do format conversion).
        for (j = 0; (j < NUM_DST_FORMATS) && (TB_OK == ret); j++)
        {
            width = (int32_t)Random_r(1.0f, blt_width);
            height = (int32_t)Random_r(1.0f, blt_height);

            error = Allocate_Buffer(&dst_buf, formats[j], width, height, 0);
            if (error != VG_LITE_SUCCESS)
            {
                VGLITE_TST_PRINTF("[%s: %d]allocate dstbuffer %d failed.", __func__, __LINE__, j);
                Free_Buffer(&src_buf);
                Free_Buffer(&dst_buf);
                return TB_BAD_API_FAIL;
            }

            xScl = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
            yScl = (vg_lite_float_t)Random_r(-2.0f, 2.0f);

            x = dst_buf.width / 2.0f;
            y = dst_buf.height / 2.0f;

            vg_lite_identity(&matrix);
            vg_lite_translate(x, y, &matrix);
            vg_lite_scale(xScl, yScl, &matrix);

            vg_lite_clear(&dst_buf, NULL, cc);
            error = vg_lite_blit(&dst_buf, &src_buf,&matrix, 0, 0, blt_filter);
            if (error) {
                VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
                Free_Buffer(&src_buf);
                Free_Buffer(&dst_buf);
                return TB_BAD_API_FAIL;
            }
            vg_lite_finish();
            
#if !defined (VGLITE_TST_FIRMWARE)
            SaveBMP_SFT("gfx_4_",&dst_buf);
#endif
            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    
    return ret;
}

/*
 Resolution: Random generated
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_blit
 Description: Generate configurable size random source buffers with gradient, then blit it to same size dest buffers.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
int     SFT_Blit_002()
{
    int ret = TB_OK;
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, nformats;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_buffer_t tempBuffer;
    int32_t width, height;
    
    nformats = (NUM_SRC_FORMATS > NUM_DST_FORMATS ? NUM_DST_FORMATS : NUM_SRC_FORMATS);
    for (i = 0; (i < nformats) && (TB_OK == ret); i++)
    {
        // Generate configurable source buffer size.
        width = (int32_t)Random_r(1.0f, blt_width);
        height = (int32_t)Random_r(1.0f, blt_height);
        
        error = Allocate_Buffer(&src_buf, formats[i], width, height, 0);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s, %d]Allocating srcBuffer %d failed.\r\n", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            return TB_BAD_API_FAIL;
        }
        error = Allocate_Buffer(&dst_buf, formats[i], width, height, 0);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s, %d]Allocating dstBuffer %d failed.\r\n", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            return TB_BAD_API_FAIL;
        }
        
        //Generate source buffer with gradient.
        error = gen_buffer(1, &src_buf, src_buf.format, src_buf.width, src_buf.height);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s, %d]generate srcBuffer %d failed.\r\n", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            return TB_BAD_GENERIC;
        }
        //Generate dest buffer with checker.
        error = gen_buffer(0, &dst_buf, dst_buf.format, dst_buf.width, dst_buf.height);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s, %d]generate dstBuffer %d failed.\r\n", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            return TB_BAD_GENERIC;
        }
        src_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
        dst_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
        
        // Construct the temp buffer to save the orginal dst.
        error = Allocate_Buffer(&tempBuffer, dst_buf.format, dst_buf.width, dst_buf.height, 0);
        
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("Allocating tempBuffer %d failed.\r\n", i);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            Free_Buffer(&tempBuffer);
            return TB_BAD_API_FAIL;
        }
        
        //Backup the dst buff.
        error = vg_lite_blit(&tempBuffer, &dst_buf, NULL, 0, 0, blt_filter);
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            Free_Buffer(&tempBuffer);
            return TB_BAD_API_FAIL;
        }
        
        //Then blit src to dest.
        error = vg_lite_blit(&dst_buf, &src_buf, NULL, 0, 0, blt_filter);
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            Free_Buffer(&tempBuffer);
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_9_10_",&dst_buf);
        
        // Save RAW file.
        if (getenv("GOLDEN") != NULL) {
            if (!strcmp(getenv("GOLDEN"), "1")) {
                if(vg_lite_save_raw(GFX_9_10_GOLDEN, &dst_buf) != 0)
                {
                    VGLITE_TST_PRINTF("save raw file error\r\n");
                    Free_Buffer(&src_buf);
                    Free_Buffer(&dst_buf);
                    Free_Buffer(&tempBuffer);
                    return TB_BAD_GENERIC;
                }
            }
        }
#endif
        
        // Check the result with golden.
        if (raw != NULL) {
            if (vg_lite_load_raw(raw, GFX_9_10_GOLDEN) != 0) {
                VGLITE_TST_PRINTF("load raw file error\r\n");
                Free_Buffer(&src_buf);
                Free_Buffer(&dst_buf);
                Free_Buffer(&tempBuffer);
                blt_cleanup();
                return TB_BAD_GENERIC;
            } else {
                if (sft_memcmp(raw->memory, (&dst_buf)->memory, (&dst_buf)->stride * (&dst_buf)->height)) {
                    VGLITE_TST_PRINTF("gfx_9_10 sft_memcmp result error\r\n");
                    Free_Buffer(&dst_buf);
                    Free_Buffer(&src_buf);
                    Free_Buffer(&tempBuffer);
                    blt_cleanup();
                    return TB_BAD_RAW_MEMCMP;
                } else {
                    VGLITE_TST_PRINTF("gfx_9_10 sft_memcmp result correct\r\n");
                    blt_cleanup();
                }
            }
        }
        
        //Restore the dst.
        error = vg_lite_blit(&dst_buf, &tempBuffer, NULL, 0, 0, blt_filter);
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            Free_Buffer(&tempBuffer);
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
        //Free buffs.
        Free_Buffer(&dst_buf);
        Free_Buffer(&tempBuffer);
        Free_Buffer(&src_buf);
    }
    
    return ret;
}

/*
 Resolution: Random generated
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_blit
 Description: Generate same random source buffers, then blit it to different strides dest buffers.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
int     SFT_Blit_003()
{
    int i;
    int ret = TB_OK;
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    int32_t    width, height;
    int loop;

    width = (int32_t)Random_r(1.0f, blt_width / 2);
    height = (int32_t)Random_r(1.0f, blt_height / 2);
    
    error = Allocate_Buffer(&src_buf, VG_LITE_RGBA8888, width, height, 0);
    if (error != VG_LITE_SUCCESS)
    {
        VGLITE_TST_PRINTF("[%s: %d]allocate srcbuff failed.\r\n", __func__, __LINE__);
        Free_Buffer(&src_buf);
        return TB_BAD_API_FAIL;
    }
    error = gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height);
    if (error != VG_LITE_SUCCESS)
    {
        VGLITE_TST_PRINTF("[%s, %d]generate srcBuffer failed.\r\n", __func__, __LINE__);
        Free_Buffer(&src_buf);
        return TB_BAD_GENERIC;
    }
    src_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    
    //Then blit src to dest (do format conversion).
    for (i = 0; (i < 2) && (TB_OK == ret); i++)
    {
        // Generate different dest buffer strides.
        error = Allocate_Buffer(&dst_buf, VG_LITE_RGBA8888, blt_width / 2, blt_height / 2, blt_width * i);
        if (error != VG_LITE_SUCCESS)
        {
            VGLITE_TST_PRINTF("[%s: %d]allocate dstbuff %d failed.\r\n", __func__, __LINE__, i);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            return TB_BAD_API_FAIL;
        }
        
        vg_lite_clear(&dst_buf, NULL, cc);
        error = vg_lite_blit(&dst_buf, &src_buf, NULL, 0, 0, blt_filter);
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
            Free_Buffer(&src_buf);
            Free_Buffer(&dst_buf);
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_11_",&dst_buf);
        
        // Save RAW file.
        if (getenv("GOLDEN") != NULL) {
            if (!strcmp(getenv("GOLDEN"), "1")) {
                if (i == 0) {
                    if(vg_lite_save_raw_byline(GFX_11_GOLDEN_1, &dst_buf) != 0)
                    {
                        VGLITE_TST_PRINTF("save raw file error\r\n");
                        Free_Buffer(&src_buf);
                        Free_Buffer(&dst_buf);
                        return TB_BAD_GENERIC;
                    }
                } else {
                    if(vg_lite_save_raw_byline(GFX_11_GOLDEN_2, &dst_buf) != 0)
                    {
                        VGLITE_TST_PRINTF("save raw file error\r\n");
                        Free_Buffer(&src_buf);
                        Free_Buffer(&dst_buf);
                        return TB_BAD_GENERIC;
                    }
                }
            }
        }
#endif
        
        // Check the result with golden.
        if (raw != NULL) {
            if (i == 0) {
                if (vg_lite_load_raw_byline(raw, GFX_11_GOLDEN_1) != 0) {
                    VGLITE_TST_PRINTF("load raw file error\r\n");
                    Free_Buffer(&src_buf);
                    Free_Buffer(&dst_buf);
                    blt_cleanup();
                    return TB_BAD_GENERIC;
                } else {
                    for (loop = 0; loop < (&dst_buf)->height; loop++) {
                        if (sft_memcmp(raw->memory, (&dst_buf)->memory, (&dst_buf)->width * ((&dst_buf)->stride / (&dst_buf)->width))) {
                            ret = TB_BAD;
                            break;
                        }
                    }
                    if (TB_BAD == ret) {
                        VGLITE_TST_PRINTF("\r\ngfx_11_1 sft_memcmp result error\r\n");
                        Free_Buffer(&dst_buf);
                        Free_Buffer(&src_buf);
                        blt_cleanup();
                        return TB_BAD_RAW_MEMCMP;
                    } else {
                        VGLITE_TST_PRINTF("\r\ngfx_11_1 sft_memcmp result correct\r\n");
                        blt_cleanup();
                    }
                }
            } else {
                if (vg_lite_load_raw_byline(raw, GFX_11_GOLDEN_2) != 0) {
                    VGLITE_TST_PRINTF("load raw file error\r\n");
                    Free_Buffer(&src_buf);
                    Free_Buffer(&dst_buf);
                    blt_cleanup();
                    return TB_BAD_GENERIC;
                } else {
                    for (loop = 0; loop < (&dst_buf)->height; loop++) {
                        if (sft_memcmp(raw->memory, (&dst_buf)->memory, (&dst_buf)->width * ((&dst_buf)->stride / (&dst_buf)->width))) {
                            ret = TB_BAD;
                            break;
                        }
                    }
                    if (TB_BAD == ret) {
                        VGLITE_TST_PRINTF("\r\ngfx_11_2 sft_memcmp result error\r\n");
                        Free_Buffer(&dst_buf);
                        Free_Buffer(&src_buf);
                        blt_cleanup();
                        return TB_BAD_RAW_MEMCMP;
                    } else {
                        VGLITE_TST_PRINTF("\r\ngfx_11_2 sft_memcmp result correct\r\n");
                        blt_cleanup();
                    }
                }
            }
        }
        
        //Free the dest buff.
        Free_Buffer(&dst_buf);
    }
    
    //Free the src buff.
    Free_Buffer(&src_buf);
    
    return ret;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
int SFT_Blit(int tst_code, vg_lite_buffer_t * raw_fb, int user_fb_width, int user_fb_height)
{
    uint32_t feature_check = 0;
    uint32_t ret = TB_OK;
    raw = raw_fb;
    blt_width = user_fb_width;
    blt_height = user_fb_height;
    
    // Initialize the blitter.
    if (vg_lite_init(user_fb_width, user_fb_height) != VG_LITE_SUCCESS)
    {
        VGLITE_TST_PRINTF("vg_lite engine initialization failed @ %s, ln%d.\r\n", __FILE__, __LINE__);
        return TB_BAD;
    }
    else
    {
        VGLITE_TST_PRINTF("vg_lite engine initialization OK.\r\n");
    }
    
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 1) {
        printf("gfx_sft_blit is not supported.\n");
        return TB_BAD;
    }
    
    InitBMP(user_fb_width, user_fb_height);

    // Set image filter type according to hardware feature.
    blt_filter = VG_LITE_FILTER_POINT;

    if (tst_code == 1) {
        VGLITE_TST_PRINTF("\r\nCase: SFT_Blit_001\r\n");
        ret = SFT_Blit_001();
    } else if (tst_code == 2) {
        VGLITE_TST_PRINTF("\r\nCase: SFT_Blit_002\r\n");
        ret = SFT_Blit_002();
    } else if (tst_code == 3) {
        VGLITE_TST_PRINTF("\r\nCase: SFT_Blit_003\r\n");
        ret = SFT_Blit_003();
    } else {
        ret = TB_BAD;
    }
    
    DestroyBMP();
    vg_lite_close();
    
    return ret;
}

