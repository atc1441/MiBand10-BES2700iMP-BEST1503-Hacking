/*-----------------------------------------------------------------------------
**Description: The test cases test the blitting of different size/formats/transformations of image.
-----------------------------------------------------------------------------*/
#include "../SFT.h"
#include "../Common.h"

vg_lite_blend_t blend_mode[BLEND_COUNT]=
{
    VG_LITE_BLEND_NONE,
    VG_LITE_BLEND_SRC_OVER,
    VG_LITE_BLEND_DST_OVER,
    VG_LITE_BLEND_SRC_IN,
    VG_LITE_BLEND_DST_IN,
    VG_LITE_BLEND_MULTIPLY,
    VG_LITE_BLEND_SCREEN,
    VG_LITE_BLEND_ADDITIVE,
    VG_LITE_BLEND_SUBTRACT,
};

/* Init buffers. */
static vg_lite_error_t gen_buffer(int type, vg_lite_buffer_t *buf, vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    void *data = NULL;
    uint8_t *pdata8, *pdest8;
    uint32_t bpp = get_bpp(format);
    int i;

    /* Randomly generate some images. */
    data = gen_image(type, format, width, height);
    pdata8 = (uint8_t*)data;
    pdest8 = (uint8_t*)buf->memory;
    if (data != NULL)
    {
        for (i = 0; i < height; i++)
        {
            memcpy (pdest8, pdata8, bpp * width / 8 );
            pdest8 += buf->stride;
            pdata8 += bpp * width / 8 ;
        }
        free(data);
    }
    else
    {
        printf("Image data generating failed.\n");
    }

    return VG_LITE_SUCCESS;
}

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}
/*-----------------------------------------------------------------------------
** init the source buffer 
-----------------------------------------------------------------------------*/
static vg_lite_error_t Init()
{
    return VG_LITE_SUCCESS;
}

/*-----------------------------------------------------------------------------
**Name: Exit
**Parameters: None
**Returned Value: None
-----------------------------------------------------------------------------*/
static void Exit()
{

}

static vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height)
{
    vg_lite_error_t error;
    /* There need to test multiple formats,so align with the mininum format width */
    memset(buffer,0,sizeof(vg_lite_buffer_t));
    buffer->width = ALIGN(width, 128);
    buffer->height = height;
    buffer->format = format;
    buffer->stride = 0;
    buffer->handle = NULL;
    buffer->memory = NULL;
    buffer->address = 0;
    buffer->tiled   = 0;

    CHECK_ERROR(vg_lite_allocate(buffer));

ErrorHandler:
    return error;
}

static void Free_Buffer(vg_lite_buffer_t *buffer)
{
    vg_lite_free(buffer);
}

/*
Test 1: Blit for different sizes between different src formats.
It blit image from src to dest, then draw both src & dest on screen.
Dst format is VG_LITE_A8.
*/
vg_lite_error_t Draw_Image_001()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i,j,k,m,n;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffa0a0a0;
    vg_lite_color_t image_cc = 0xff00ffff;
    int32_t    src_width, src_height, dst_width, dst_height;
    char name[]="Draw_Image_001_%d_";
    BOOL save_result = TRUE;
    vg_lite_buffer_format_t src_formats[SRC_FORMATS_CAT_COUNT] =
    {
        VG_LITE_BGRA8888,
        VG_LITE_BGRA4444,
        VG_LITE_BGRA2222,
        VG_LITE_BGRX8888,
        VG_LITE_BGRA5551,
        VG_LITE_BGR565,
        VG_LITE_A8,
        VG_LITE_A4,
        VG_LITE_L8,
    };
    vg_lite_buffer_format_t dst_formats[DST_FORMATS_CAT_COUNT] =
    {
        VG_LITE_BGRA8888,
        VG_LITE_BGRA4444,
        VG_LITE_BGRA2222,
        VG_LITE_BGRX8888,
        VG_LITE_BGRA5551,
        VG_LITE_BGR565,
        VG_LITE_A8,
        VG_LITE_L8,
    };
    vg_lite_buffer_image_mode_t image_modes[IMAGEMODEL_COUNT] =
    {
        VG_LITE_NONE_IMAGE_MODE,
        VG_LITE_NORMAL_IMAGE_MODE,
        VG_LITE_MULTIPLY_IMAGE_MODE
    };
    vg_lite_filter_t filter_modes[FILTER_COUNT] =
    {
        VG_LITE_FILTER_POINT,
        VG_LITE_FILTER_LINEAR,
        VG_LITE_FILTER_BI_LINEAR,
    };
    vg_lite_blend_t blend_modes[BLEND_COUNT]=
    {
        VG_LITE_BLEND_NONE,
        VG_LITE_BLEND_SRC_OVER,
        VG_LITE_BLEND_DST_OVER,
        VG_LITE_BLEND_SRC_IN,
        VG_LITE_BLEND_DST_IN,
        VG_LITE_BLEND_MULTIPLY,
        VG_LITE_BLEND_SCREEN,
        VG_LITE_BLEND_ADDITIVE,
        VG_LITE_BLEND_SUBTRACT,
    };

    for (i = 0; i < SRC_FORMATS_CAT_COUNT; i++) {
        sprintf(name,"Draw_Image_001_%d_",i);
        for (j = 0; j < DST_FORMATS_CAT_COUNT; j++)
            for (k = 0; k < IMAGEMODEL_COUNT; k++)
                for (m = 0; m < FILTER_COUNT; m++)
                    for (n = 0; n < BLEND_COUNT; n++)
                    {
                        if(src_formats[i] == VG_LITE_BGRA2222 || dst_formats[j] ==VG_LITE_BGRA2222)
                        {
                             if(!vg_lite_query_feature(gcFEATURE_BIT_VG_RGBA2_FORMAT))
                             {
                                 save_result = FALSE;
                             }
                        }
                        
                        src_width = (int32_t)WINDSIZEX;
                        src_height = (int32_t)WINDSIZEY;
                        dst_width = (int32_t)WINDSIZEX;
                        dst_height = (int32_t)WINDSIZEY;

                        CHECK_ERROR(Allocate_Buffer(&src_buf, src_formats[i], src_width, src_height));
                        CHECK_ERROR(gen_buffer(i % 2, &src_buf, src_formats[i], src_buf.width, src_buf.height));
                        CHECK_ERROR(Allocate_Buffer(&dst_buf, dst_formats[j], dst_width, dst_height));
                        CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
                        src_buf.image_mode = image_modes[k];
                        CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, blend_modes[n], image_cc, filter_modes[m]));
                        CHECK_ERROR(vg_lite_finish());
                        SaveBMP_SFT(name,&dst_buf,save_result);
                        /* Free the dest buff. */
                        Free_Buffer(&dst_buf);

                        /* Free the src buff. */
                        Free_Buffer(&src_buf);
                        save_result = TRUE;
                    }
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    if(src_buf.handle != NULL)
        Free_Buffer(&src_buf);
    return error;
}
vg_lite_error_t Draw_Image_002()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i,j,k,m,n;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffa0a0a0;
    vg_lite_color_t image_cc = 0xff00ffff;
    BOOL save_result = TRUE;
    int32_t    src_width, src_height, dst_width, dst_height;
    vg_lite_buffer_format_t src_formats[SRC_FORMATS_COUNT] =
    {
        VG_LITE_BGRA8888,
        VG_LITE_RGBA8888,
        VG_LITE_ABGR8888,
        VG_LITE_ARGB8888,
        VG_LITE_BGRA4444,
        VG_LITE_RGBA4444,
        VG_LITE_ABGR4444,
        VG_LITE_ARGB4444,
        VG_LITE_BGRA2222,
        VG_LITE_RGBA2222,
        VG_LITE_ABGR2222,
        VG_LITE_ARGB2222,
        VG_LITE_BGRX8888,
        VG_LITE_RGBX8888,
        VG_LITE_XRGB8888,
        VG_LITE_XBGR8888,
        VG_LITE_BGRA5551,
        VG_LITE_RGBA5551,
        VG_LITE_ARGB1555,
        VG_LITE_ABGR1555,
        VG_LITE_BGR565,
        VG_LITE_RGB565,
        VG_LITE_A8,
        VG_LITE_A4,
        VG_LITE_L8,
    };
    vg_lite_buffer_format_t dst_formats[DST_FORMATS_COUNT] =
    {
        VG_LITE_BGRA8888,
        VG_LITE_RGBA8888,
        VG_LITE_ABGR8888,
        VG_LITE_ARGB8888,
        VG_LITE_BGRA4444,
        VG_LITE_RGBA4444,
        VG_LITE_ABGR4444,
        VG_LITE_ARGB4444,
        VG_LITE_BGRA2222,
        VG_LITE_RGBA2222,
        VG_LITE_ABGR2222,
        VG_LITE_ARGB2222,
        VG_LITE_BGRX8888,
        VG_LITE_RGBX8888,
        VG_LITE_XRGB8888,
        VG_LITE_XBGR8888,
        VG_LITE_BGRA5551,
        VG_LITE_RGBA5551,
        VG_LITE_ARGB1555,
        VG_LITE_ABGR1555,
        VG_LITE_BGR565,
        VG_LITE_RGB565,
        VG_LITE_A8,
        VG_LITE_L8,
    };
    for (i = 0; i < SRC_FORMATS_COUNT; i++)
        for (j = 0; j < DST_FORMATS_COUNT; j++)
        {
            src_width = (int32_t)WINDSIZEX;
            src_height = (int32_t)WINDSIZEY;
            dst_width = (int32_t)WINDSIZEX;
            dst_height = (int32_t)WINDSIZEY;
            if(src_formats[i] == VG_LITE_BGRA2222 || src_formats[i] == VG_LITE_RGBA2222 || src_formats[i] == VG_LITE_ABGR2222 || src_formats[i] == VG_LITE_ARGB2222
               || dst_formats[j] ==VG_LITE_BGRA2222 || dst_formats[j] == VG_LITE_RGBA2222 || dst_formats[j] == VG_LITE_ABGR2222 || dst_formats[j] == VG_LITE_ARGB2222)
           {
                if(!vg_lite_query_feature(gcFEATURE_BIT_VG_RGBA2_FORMAT))
               {
                      save_result = FALSE;
                }
            }
            CHECK_ERROR(Allocate_Buffer(&src_buf, src_formats[i], src_width, src_height));
            CHECK_ERROR(gen_buffer(i % 2, &src_buf, src_formats[i], src_buf.width, src_buf.height));
            CHECK_ERROR(Allocate_Buffer(&dst_buf, dst_formats[j], dst_width, dst_height));
            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, VG_LITE_BLEND_NONE, image_cc, VG_LITE_FILTER_POINT));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("Draw_Image_002",&dst_buf,save_result);
            /* Free the dest buff. */
            Free_Buffer(&dst_buf);

            /* Free the src buff. */
            Free_Buffer(&src_buf);
            save_result = TRUE;
        }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    if(src_buf.handle != NULL)
        Free_Buffer(&src_buf);
    return error;
}
/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Draw_Image()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Blit:::::::::::::::::::::Started\n");
    if (VG_LITE_SUCCESS == Init())
    {
        output_string("\nCase: Draw_Image_001:::::::::::::::::::::Started\n");
        CHECK_ERROR(Draw_Image_001());
        output_string("\nCase: Draw_Image_001:::::::::::::::::::::Ended\n");
        
        output_string("\nCase: Draw_Image_002:::::::::::::::::::::Started\n");
        CHECK_ERROR(Draw_Image_002());
        output_string("\nCase: Draw_Image_002:::::::::::::::::::::Ended\n");
    }
    output_string("\nCase: Blit:::::::::::::::::::::Ended\n");

ErrorHandler:
    Exit();
    return error;
}


/* ***
* Logging entry
*/
void Draw_Image_Log()
{
}

