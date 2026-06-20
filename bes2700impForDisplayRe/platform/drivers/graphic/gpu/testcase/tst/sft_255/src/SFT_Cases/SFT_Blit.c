//-----------------------------------------------------------------------------
//Description: The test cases test the blitting of different size/formats/transformations of image.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

#define     NUM_SRC_FORMATS 10
#define     NUM_DST_FORMATS 10
#define     NUM_BLEND_MODES 9

/* Format array. */
static vg_lite_buffer_format_t formats[] =
{
    VG_LITE_RGBA8888,
    VG_LITE_BGRA8888,
    VG_LITE_RGBX8888,
    VG_LITE_BGRX8888,
    VG_LITE_RGB565,
    VG_LITE_BGR565,
    VG_LITE_RGBA4444,
    VG_LITE_BGRA4444,
    VG_LITE_A8,
    VG_LITE_L8,
    VG_LITE_YUYV
};


vg_lite_blend_t blend_mode[]=
{
    VG_LITE_BLEND_NONE,
    VG_LITE_BLEND_SRC_OVER,
    VG_LITE_BLEND_DST_OVER,
    VG_LITE_BLEND_SRC_IN,
    VG_LITE_BLEND_DST_IN,
    VG_LITE_BLEND_SCREEN,
    VG_LITE_BLEND_MULTIPLY,
    VG_LITE_BLEND_ADDITIVE,
    VG_LITE_BLEND_SUBTRACT
};

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

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}
//-----------------------------------------------------------------------------
// init the source buffer 
//-----------------------------------------------------------------------------
static vg_lite_error_t Init()
{
    return VG_LITE_SUCCESS;
}

//-----------------------------------------------------------------------------
//Name: Exit
//Parameters: None
//Returned Value: None
//-----------------------------------------------------------------------------
static void Exit()
{

}

static vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height)
{
    vg_lite_error_t error;
    memset(buffer,0,sizeof(vg_lite_buffer_t));
    buffer->width = ALIGN(width,128);
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
Test 1: Blit for different sizes between different src/dst formats.
It blit image from src to dest
*/
vg_lite_error_t SFT_Blit_001()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    int32_t    width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        error = Allocate_Buffer(&src_buf, formats[i], width, height);
        if (error)
        {
            printf("[%s]Allocate_Buffer %d failed. error type is %s\n", __func__, __LINE__,error_type[error]);
            return error;
        }
        error = gen_buffer(i % 2, &src_buf, formats[i], src_buf.width, src_buf.height);
        if (error)
        {
            printf("[%s]gen_buffer %d failed. error type is %s\n", __func__, __LINE__,error_type[error]);
            Free_Buffer(&src_buf);
            return error;
        }
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_001_",&src_buf);

        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);

            error = Allocate_Buffer(&dst_buf, formats[j], width, height);
            if (error)
            {
                printf("[%s]Allocate_Buffer %d failed. error type is %s\n", __func__, __LINE__,error_type[error]);
                Free_Buffer(&src_buf);
                return error;
            }

            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, (vg_lite_blend_t)0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_001_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free the dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free the src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}

//different format, width scale matrix, no blending
vg_lite_error_t SFT_Blit_002()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    float x, y;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    vg_lite_matrix_t matrix;
    vg_lite_float_t xScl, yScl;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));
        //Regenerate src buffers. Check is good for transformation.
        error = gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height);
        if (error)
        {
            printf("[%s]gen_buffer %d failed. error type is %s\n", __func__, __LINE__,error_type[error]);
            Free_Buffer(&src_buf);
            return error;
        }

        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_002_",&src_buf);

        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);

            error = Allocate_Buffer(&dst_buf, formats[j], width, height);

            xScl = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
            yScl = (vg_lite_float_t)Random_r(-2.0f, 2.0f);

            x = dst_buf.width / 2.0f;
            y = dst_buf.height / 2.0f;

            vg_lite_identity(&matrix);
            vg_lite_translate(x, y, &matrix);
            vg_lite_scale(xScl, yScl, &matrix);
            printf("blit src to dst with scale: xScl = %f, yScl = %f\n",xScl,yScl);

            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf,&matrix, (vg_lite_blend_t)0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_002_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}
//different format, width rotate matrix, no blending
vg_lite_error_t SFT_Blit_003()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    float x, y;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    vg_lite_matrix_t matrix;
    vg_lite_float_t degrees;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));

        //Regenerate src buffers. Check is good for transformation.
        CHECK_ERROR(gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height));
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_003_",&src_buf);

        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);
            CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[j], width, height));

            degrees = (vg_lite_float_t)Random_r(-360.0f, 360.0f);
            x = dst_buf.width / 2.0f;
            y = dst_buf.height / 2.0f;

            vg_lite_identity(&matrix);
            //Put it to the center of dest so that we can observe the result.
            vg_lite_translate(x, y, &matrix);
            vg_lite_rotate(degrees, &matrix);
            printf("blit with rotation: %f degrees.\n", degrees);

            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, &matrix, (vg_lite_blend_t)0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_003_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}

//different format, width translate matrix, no blending
vg_lite_error_t SFT_Blit_004()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_matrix_t matrix;
    vg_lite_float_t xOffs, yOffs;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));

        //Regenerate src buffers. Check is good for transformation.
        CHECK_ERROR(gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height));
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_004_",&src_buf);

        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);

            CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[j], width, height));

            xOffs = (vg_lite_float_t)Random_r(-src_buf.width, dst_buf.width);
            yOffs = (vg_lite_float_t)Random_r(-src_buf.height, dst_buf.height);

            vg_lite_identity(&matrix);
            vg_lite_translate(xOffs, yOffs, &matrix);
            printf("blit with translation: %f, %f\n", xOffs, yOffs);

            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, &matrix, (vg_lite_blend_t)0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_004_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}
//different format, width perspective matrix, no blending
vg_lite_error_t SFT_Blit_005()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with WHITE.
    vg_lite_matrix_t matrix;
    vg_lite_float_t w0, w1;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));

        //Regenerate src buffers. Check is good for transformation.
        CHECK_ERROR(gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height));
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_005_",&src_buf);

        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);

            CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[j], width, height));

            w0 = (vg_lite_float_t)Random_r(0.0001f, 0.01f);
            w1 = (vg_lite_float_t)Random_r(0.0001f, 0.01f);

            vg_lite_identity(&matrix);
            vg_lite_perspective(w0, w1, &matrix);

            printf("blit with perspective factor: %f, %f\n", w0, w1);

            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, &matrix, (vg_lite_blend_t)0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_005_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}

//combined transformation including of scale, rotate, translate and perspective, no blending
vg_lite_error_t SFT_Blit_006()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_matrix_t matrix;
    vg_lite_float_t sx, sy, tx, ty, degrees, w0, w1;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));

        //Regenerate src buffers. Check is good for transformation.
        CHECK_ERROR(gen_buffer(0, &src_buf, formats[i], src_buf.width, src_buf.height));
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_006_",&src_buf);
        //Then blit src to dest (do format conversion).
        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);

            CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[j], width, height));

            sx = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
            sy = (vg_lite_float_t)Random_r(-2.0f, 2.0f);

            tx = (vg_lite_float_t)dst_buf.width / 2.0f;
            ty = (vg_lite_float_t)dst_buf.height/ 2.0f;

            degrees = (vg_lite_float_t)Random_r(-360.0f, 360.0f);

            w0 = (vg_lite_float_t)Random_r(0.001f, 0.01f);
            w1 = (vg_lite_float_t)Random_r(0.001f, 0.01f);

            vg_lite_identity(&matrix);
            //scale,rotate,translate,perspective transform
            vg_lite_translate(tx, ty, &matrix);
            vg_lite_rotate(degrees, &matrix);
            vg_lite_scale(sx, sy, &matrix);
            vg_lite_perspective(w0, w1, &matrix);

            printf("blit rotate: %f, trans: %f, %f, perspective: %f, %f, scale: %f, %f\n",
                degrees, tx, ty, w0, w1, sx, sy);
            CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, cc));
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, &matrix, 0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_006_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
            vg_lite_finish();

            //Free dest buff.
            Free_Buffer(&dst_buf);
        }

        //Free src buff.
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    return error;
}

//same format, same size,different blending modes
vg_lite_error_t SFT_Blit_007()
{
    int i, k;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_buffer_t srcbuffer;
    vg_lite_buffer_t dstbuffer;
    int32_t width, height;

    for (i = 0; i < NUM_BLEND_MODES; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);
        k = 0;//(int32_t)Random_r(0.0f, NUM_SRC_FORMATS);
        CHECK_ERROR(Allocate_Buffer(&srcbuffer, formats[k], width, height));

        //vg_lite_clear(&srcbuffer[i],NULL,0xffffff00);
        //Generate the src buffer with gradient.
        CHECK_ERROR(gen_buffer(1, &srcbuffer, formats[k], srcbuffer.width, srcbuffer.height));

        CHECK_ERROR(Allocate_Buffer(&dstbuffer, formats[k], width, height));
        //vg_lite_clear(&dstbuffer[i],NULL,cc);
        //Generate the dst buffer with checker.
        CHECK_ERROR(gen_buffer(0, &dstbuffer, formats[k], dstbuffer.width, dstbuffer.height));

        //Save src & buffer.
        SaveBMP_SFT("SFT_Blit_007_",&srcbuffer);
        SaveBMP_SFT("SFT_Blit_007_",&dstbuffer);

        printf("blit with format: %d, blend mode: %d\n", formats[k], blend_mode[i]);

        //Then blit src to dest (do format conversion).
        CHECK_ERROR(vg_lite_blit(&dstbuffer, &srcbuffer, NULL, blend_mode[i], 0, filter));
        CHECK_ERROR(vg_lite_finish());

        SaveBMP_SFT("SFT_Blit_007_",&dstbuffer);

        vg_lite_blit(g_fb, &dstbuffer, NULL, 0, 0, filter);
        vg_lite_finish();

        Free_Buffer(&srcbuffer);
        Free_Buffer(&dstbuffer);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dstbuffer.handle != NULL) {
        vg_lite_free(&dstbuffer);
    }
    if (srcbuffer.handle != NULL) {
        vg_lite_free(&srcbuffer);
    }
    return error;
}

//different formats,same size, different blending modes
vg_lite_error_t SFT_Blit_008()
{
    int i, j, k;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_buffer_t srcbuffer;
    vg_lite_buffer_t dstbuffer;
    vg_lite_buffer_t tempBuffer;
    int32_t width, height;

    width = (int32_t)Random_r(1.0f, WINDSIZEX);
    height = (int32_t)Random_r(1.0f, WINDSIZEY);

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        CHECK_ERROR(Allocate_Buffer(&srcbuffer, formats[i], width, height));

        //Generate src with gradient.
        CHECK_ERROR(gen_buffer(1, &srcbuffer/*[i]*/, formats[i], srcbuffer.width, srcbuffer.height));
        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_008_",&srcbuffer);

        for (j = 0; j < NUM_DST_FORMATS; j++)
        {
            if (i == j)
                continue;
            CHECK_ERROR(Allocate_Buffer(&dstbuffer, formats[j], width, height));
            CHECK_ERROR(Allocate_Buffer(&tempBuffer, dstbuffer.format, dstbuffer.width, dstbuffer.height));

            //Generate dst with checkers.
            CHECK_ERROR(gen_buffer(0, &dstbuffer, formats[j], dstbuffer.width, dstbuffer.height));

            //Backup the dstbuffer.
            CHECK_ERROR(vg_lite_blit(&tempBuffer, &dstbuffer, NULL, 0, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_008_",&dstbuffer);

            for(k = 0; k < NUM_BLEND_MODES; k++)
            {
                //Then blit src to dest (do format conversion).
                CHECK_ERROR(vg_lite_blit(&dstbuffer, &srcbuffer, NULL, blend_mode[k], 0, filter));
                CHECK_ERROR(vg_lite_finish());
                SaveBMP_SFT("SFT_Blit_008_",&dstbuffer);

                vg_lite_blit(g_fb, &dstbuffer, NULL, 0, 0, filter);

                //Restore dstbuffer.
                vg_lite_blit(&dstbuffer, &tempBuffer, NULL, 0, 0, filter);
                vg_lite_finish();
            }

            //Free dest & temp buff.
            Free_Buffer(&dstbuffer);
            Free_Buffer(&tempBuffer);
        }

        //Free src buff.
        Free_Buffer(&srcbuffer);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dstbuffer.handle != NULL) {
        vg_lite_free(&dstbuffer);
    }
    if (srcbuffer.handle != NULL) {
        vg_lite_free(&srcbuffer);
    }
    if (tempBuffer.handle != NULL) {
        vg_lite_free(&tempBuffer);
    }
    return error;
}

//same format, different size,different blending modes
vg_lite_error_t SFT_Blit_009()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j, nformats;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_buffer_t tempBuffer;
    int32_t width, height;

    nformats = (NUM_SRC_FORMATS > NUM_DST_FORMATS ? NUM_DST_FORMATS : NUM_SRC_FORMATS);
    for (i = 0; i < nformats; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);

        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));
        CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[i], width, height));

        //Generate src buffer with gradient, dst buffer with checker.
        CHECK_ERROR(gen_buffer(1, &src_buf, src_buf.format, src_buf.width, src_buf.height));
        CHECK_ERROR(gen_buffer(0, &dst_buf, dst_buf.format, dst_buf.width, dst_buf.height));

        //Save src buffer.
        SaveBMP_SFT("SFT_Blit_009_",&src_buf);
        SaveBMP_SFT("SFT_Blit_009_",&dst_buf);

        //Construct the temp buffer to save the orginal dst.
        CHECK_ERROR(Allocate_Buffer(&tempBuffer, dst_buf.format, dst_buf.width, dst_buf.height));

        //Backup the dst buff.
        CHECK_ERROR(vg_lite_blit(&tempBuffer, &dst_buf, NULL, 0, 0, filter));
        //Then blit src to dest with different blending modes.
        for (j = 0; j < NUM_BLEND_MODES; j++)
        {
            CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, blend_mode[j], 0, filter));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("SFT_Blit_009_",&dst_buf);

            vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

            //Restore the dst.
            vg_lite_blit(&dst_buf, &tempBuffer, NULL, 0, 0, filter);
            vg_lite_finish();
        }

        //Free buffs.
        Free_Buffer(&dst_buf);
        Free_Buffer(&tempBuffer);
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    if (tempBuffer.handle != NULL) {
        vg_lite_free(&tempBuffer);
    }
    return error;
}

// different size, different blending modes, different formats
vg_lite_error_t SFT_Blit_010()
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int i, j, k;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc = 0xffffffff;    //Clear with BLACK.
    vg_lite_buffer_t    tempBuffer;
    int32_t width, height;

    for (i = 0; i < NUM_SRC_FORMATS; i++)
    {
        width = (int32_t)Random_r(1.0f, WINDSIZEX);
        height = (int32_t)Random_r(1.0f, WINDSIZEY);
        CHECK_ERROR(Allocate_Buffer(&src_buf, formats[i], width, height));
        CHECK_ERROR(gen_buffer(1, &src_buf, src_buf.format, src_buf.width, src_buf.height));

        //Save src.

        SaveBMP_SFT("SFT_Blit_010_",&src_buf);
        for (j = 0;j < NUM_DST_FORMATS; j++)
        {
            width = (int32_t)Random_r(1.0f, WINDSIZEX);
            height = (int32_t)Random_r(1.0f, WINDSIZEY);
            CHECK_ERROR(Allocate_Buffer(&dst_buf, formats[j], width, height));
            CHECK_ERROR(gen_buffer(1, &dst_buf, dst_buf.format, dst_buf.width, dst_buf.height));

            //Backup dst_buf.
            CHECK_ERROR(Allocate_Buffer(&tempBuffer, dst_buf.format, dst_buf.width, dst_buf.height));
            CHECK_ERROR(vg_lite_blit(&tempBuffer, &dst_buf, NULL, VG_LITE_BLEND_NONE, 0, filter));
            CHECK_ERROR(vg_lite_finish());
            //Save dst buf.
            SaveBMP_SFT("SFT_Blit_010_",&dst_buf);
            for(k = 0;k < NUM_BLEND_MODES; k++)
            {
                //Then blit src to dest (do format conversion).
                CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, blend_mode[k], 0, filter));
                CHECK_ERROR(vg_lite_finish());
                SaveBMP_SFT("SFT_Blit_010_",&dst_buf);

                vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

                //Restore dst.
                vg_lite_blit(&dst_buf, &tempBuffer, NULL, VG_LITE_BLEND_NONE, 0, filter);
                vg_lite_finish();
            }

            //Free dst & temp buff.
            Free_Buffer(&dst_buf);
            Free_Buffer(&tempBuffer);
        }
        Free_Buffer(&src_buf);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if (dst_buf.handle != NULL) {
        vg_lite_free(&dst_buf);
    }
    if (src_buf.handle != NULL) {
        vg_lite_free(&src_buf);
    }
    if (tempBuffer.handle != NULL) {
        vg_lite_free(&tempBuffer);
    }
    return error;
}


/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t SFT_Blit()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Blit:::::::::::::::::::::Started\n");
    if (VG_LITE_SUCCESS == Init())
    {
        output_string("\nCase: SFT_Blit_001:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_001());
        output_string("\nCase: SFT_Blit_001:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_002:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_004());
        output_string("\nCase: SFT_Blit_002:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_003:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_003());
        output_string("\nCase: SFT_Blit_003:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_004:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_005());
        output_string("\nCase: SFT_Blit_004:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_005:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_002());
        output_string("\nCase: SFT_Blit_005:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_006:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_008());
        output_string("\nCase: SFT_Blit_006:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_007:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_007());
        output_string("\nCase: SFT_Blit_007:::::::::::::::::::::Ended\n");
        
        output_string("\nCase: SFT_Blit_008:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_006());
        output_string("\nCase: SFT_Blit_008:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_009:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_009());
        output_string("\nCase: SFT_Blit_009:::::::::::::::::::::Ended\n");

        output_string("\nCase: SFT_Blit_010:::::::::::::::::::::Started\n");
        CHECK_ERROR(SFT_Blit_010());
        output_string("\nCase: SFT_Blit_010:::::::::::::::::::::Ended\n");

    }
    output_string("\nCase: Blit:::::::::::::::::::::Ended\n");

ErrorHandler:
    Exit();
    return error;
}


/* ***
* Logging entry
*/
void SFT_Blit_Log()
{
}

