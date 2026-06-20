//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"


/* Buffers for Combination tests. */
vg_lite_buffer_t src_buf;
static vg_lite_buffer_t dst_buf;
vg_lite_path_t path;
void *path_data;

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}
/*-----------------------------------------------------------------------------
**Name: Init
**Parameters: None
**Returned Value: None
**Description: Create a dest buffer object
------------------------------------------------------------------------------*/
static vg_lite_error_t Init(int width, int height)
{
    int32_t src_width, src_height;
    vg_lite_error_t error;
    
    src_width = (int)width;
    src_height = (int)height;
    memset(&src_buf,0,sizeof(vg_lite_buffer_t));
    src_buf.width   = src_width;
    src_buf.height  = src_height;
    src_buf.format  = VG_LITE_RGBA8888;
    src_buf.stride  = 0;
    src_buf.handle  = NULL;
    src_buf.memory  = NULL;
    src_buf.address = 0;
    src_buf.tiled   = 0;
    
    CHECK_ERROR(vg_lite_allocate(&src_buf));
    memset(&dst_buf,0,sizeof(vg_lite_buffer_t));
    dst_buf.width   = width;
    dst_buf.height  = height;
    dst_buf.format  = VG_LITE_RGBA8888;
    dst_buf.stride  = 0;
    dst_buf.handle  = NULL;
    dst_buf.memory  = NULL;
    dst_buf.address = 0;
    dst_buf.tiled   = 0;
    
    CHECK_ERROR(vg_lite_allocate(&dst_buf));
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(src_buf.handle != NULL)
        vg_lite_free(&src_buf);
    if(dst_buf.handle != NULL)
        vg_lite_free(&dst_buf);
    return error;
}

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

/* Construct star path. */
void draw_star()
{
    uint8_t *opcode;
    uint8_t *pdata8;
    uint32_t offset;
    uint32_t path_length;
    uint8_t opfstart[] = { MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_DONE};
    
    /*
     Test path 1: A five start.
     */
    path_length = 4 * 5 + 1 + 8 * 5;
    path_data = malloc(path_length);
    opcode = (uint8_t*)path_data;
    pdata8 = (uint8_t*)path_data;
    offset = 0;
    
    /* opcode 0: MOVE, 2 floats */
    *(opcode + offset) = opfstart[0];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 50.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 100.0f;
    offset += 4;
    
    /* opcode 1: LINE, 2 floats. */
    *(opcode + offset) = opfstart[1];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 300.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -10.0f;
    offset += 4;
    
    /* opcode 2: LINE, 2 floats. */
    *(opcode + offset) = opfstart[2];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -250.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 200.0f;
    offset += 4;
    
    /* opcode 3: LINE, 2 floats. */
    *(opcode + offset) = opfstart[3];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 80.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -280.0f;
    offset += 4;
    
    /* opcode 4: LINE, 2 floats. */
    *(opcode + offset) = opfstart[4];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 80.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 260.0f;
    offset += 4;
    
    /* opcode 5: END. */
    *(opcode + offset) = opfstart[5];
    offset++;
    
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, WINDSIZEX, WINDSIZEY);
}

vg_lite_error_t Combination_001()
{
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);

    Init(320, 240);

    draw_star();

    /* Draw the path and blit src to dest. */

    CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height));
    /* Save src buffer. */
    SaveBMP_SFT("Combination_001_",&src_buf,TRUE);

    Clear_Screen(0);
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_BI_LINEAR));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("Combination_001_", &dst_buf,TRUE);

ErrorHandler:
    if(src_buf.handle != NULL)
        vg_lite_free(&src_buf);
    if(dst_buf.handle != NULL)
        vg_lite_free(&dst_buf);
    if(path_data != NULL)
        free(path_data);
    path.path = NULL;
    return error;
}

vg_lite_error_t Combination_002()
{
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);

    Init(320, 240);

    draw_star();

    /* Draw the path and blit src to dest. */

    CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height));
    /* Save src buffer. */
    SaveBMP_SFT("Combination_002_",&src_buf,TRUE);

    Clear_Screen(0);
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, VG_LITE_BLEND_MULTIPLY, 0, VG_LITE_FILTER_BI_LINEAR));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("Combination_002_", &dst_buf,TRUE);

ErrorHandler:
    if(src_buf.handle != NULL)
        vg_lite_free(&src_buf);
    if(dst_buf.handle != NULL)
        vg_lite_free(&dst_buf);
    if(path_data != NULL)
        free(path_data);
    path.path = NULL;
    return error;
}

vg_lite_error_t Combination_003()
{
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);

    Init(1920, 1080);

    draw_star();

    /* Draw the path and blit src to dest. */

    CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height));
    /* Save src buffer. */
    SaveBMP_SFT("Combination_003_",&src_buf,TRUE);

    Clear_Screen(0);
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_BI_LINEAR));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("Combination_003_", &dst_buf,TRUE);

ErrorHandler:
    if(src_buf.handle != NULL)
        vg_lite_free(&src_buf);
    if(dst_buf.handle != NULL)
        vg_lite_free(&dst_buf);
    if(path_data != NULL)
        free(path_data);
    path.path = NULL;
    return error;
}

vg_lite_error_t Combination_004()
{
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);

    Init(1920, 1080);

    draw_star();

    /* Draw the path and blit src to dest. */

    CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height));
    /* Save src buffer. */
    SaveBMP_SFT("Combination_004_",&src_buf,TRUE);

    Clear_Screen(0);
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, NULL, VG_LITE_BLEND_MULTIPLY, 0, VG_LITE_FILTER_BI_LINEAR));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("Combination_004_", &dst_buf,TRUE);

ErrorHandler:
    if(src_buf.handle != NULL)
        vg_lite_free(&src_buf);
    if(dst_buf.handle != NULL)
        vg_lite_free(&dst_buf);
    if(path_data != NULL)
        free(path_data);
    path.path = NULL;
    return error;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Combination()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Combination:::::::::::::::::::::Started\n");

    output_string("\nCase: Combination_001:::::::::Started\n");
    CHECK_ERROR(Combination_001());
    output_string("\nCase: Combination_001:::::::::Ended\n");
    output_string("\nCase: Combination_002:::::::::Started\n");
    CHECK_ERROR(Combination_002());
    output_string("\nCase: Combination_002:::::::::Ended\n");
    output_string("\nCase: Combination_003:::::::::Started\n");
    CHECK_ERROR(Combination_003());
    output_string("\nCase: Combination_003:::::::::Ended\n");
    output_string("\nCase: Combination_004:::::::::Started\n");
    CHECK_ERROR(Combination_004());
    output_string("\nCase: Combination_004:::::::::Ended\n");

    output_string("\nCase: Combination:::::::::::::::::::::Ended\n");

ErrorHandler:
    return error;
}


/* ***
* Logging entry
*/
void Combination_Log()
{
}

