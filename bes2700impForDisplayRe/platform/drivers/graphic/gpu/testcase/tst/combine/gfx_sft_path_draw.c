//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "SFT.h"

#define COUNT 1        //Num paths to test.

static vg_lite_buffer_t * raw = NULL;
static int draw_width = 0;
static int draw_height = 0;

/* Buffers for path_draw tests. */
static vg_lite_buffer_t dst_buf = {0};

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

static int data_count[] =
{
0,
0,
2,
2,
2,
2,
4,
4,
6,
6
};

static void draw_cleanup(void)
{
    if (raw->handle != NULL) {
        // Free the raw memory.
        vg_lite_free(raw);
    }
}

//-----------------------------------------------------------------------------
//Name: Init
//Parameters: None
//Returned Value: None
//Description: Create a dest buffer object
//-----------------------------------------------------------------------------
static vg_lite_error_t Init()
{
    vg_lite_error_t error;

    dst_buf.width = draw_width;
    dst_buf.height = draw_height;
    dst_buf.format = formats[0];
    dst_buf.stride = 0;
    dst_buf.handle = NULL;
    dst_buf.memory = NULL;
    dst_buf.address = 0;
    dst_buf.tiled = 0;

    error = vg_lite_allocate(&dst_buf);

    if (error != VG_LITE_SUCCESS)
    {
        // Roll back.
        vg_lite_free(&dst_buf);
        VGLITE_TST_PRINTF("allocate dst_buf failed");
        return TB_BAD_API_FAIL;
    }
    else
    {
        VGLITE_TST_PRINTF("dst buffer initialize success");
    }
    
    return VG_LITE_SUCCESS;
}

//-----------------------------------------------------------------------------
//Name: Exit
//Parameters: None
//Returned Value: None
//Description: Destroy the buffer objects created in Init() function
//-----------------------------------------------------------------------------
static void Exit()
{
    vg_lite_free(&dst_buf);
}

#define ALIGN(size, align) \
    (((size) + (align) - 1) / (align) * (align))

int32_t calc_data_size(
                       uint8_t * opcodes,
                       int       count,
                       vg_lite_format_t format
                       )
{
    int i;
    int32_t size = 0;
    int data_size = 0;
    uint8_t *pdata = opcodes;

    switch (format)
    {
    case VG_LITE_S8:    data_size = 1;  break;
    case VG_LITE_S16:   data_size = 2;  break;
    case VG_LITE_S32:
    case VG_LITE_FP32:  data_size = 4;  break;
    default:    break;
    }
    for (i = 0; i < count; i++)
    {
        //Opcode size.
        size++;

        //Data size.
        if ((data_size > 1) &&          //Need alignment.
            (data_count[*pdata] > 0))   //Has data.
        {
            size = ALIGN(size, data_size);
        }
        size += data_count[*pdata] * data_size;
        pdata++;
    }

    return size;
}

int fill_path_data(
                   vg_lite_format_t    format,
                   int                 opcount,
                   uint8_t *           opcodes,
                   void    *           path_data,
                   int                 num_subs)
{
    int offset_path;        //Pointer offset in path data.
    uint8_t     * pcode;
    int8_t      * path_data8;
    int16_t     * path_data16;
    int32_t     * path_data32;
    float       * path_datafp;
    int         i,j;
    int         data_size = 0;
    float       low;
    float       hi;
    float       rel_low;
    float       rel_hi;
    float       step;

    hi  = draw_width / 2.0f;
    low = -hi;
    rel_low = -30.0f;
    rel_hi  = 30.0f;
    step = (hi - low) / num_subs;

    switch (format)
    {
    case VG_LITE_S8:    data_size = 1;  break;
    case VG_LITE_S16:   data_size = 2;  break;
    case VG_LITE_S32:
    case VG_LITE_FP32:  data_size = 4;  break;
    default:    break;
    }

    pcode = opcodes;
    path_data8  = (int8_t*)  path_data;
    path_data16 = (int16_t*) path_data;
    path_data32 = (int32_t*) path_data;
    path_datafp = (float*)    path_data;
    offset_path = 0;

    hi = low + step;
    for (i = 0; i < opcount; i++)
    {
        *path_data8 = *pcode;       //Copy the opcode.
        offset_path++;
        if (data_size > 1 &&            //Need alignment
            data_count[*pcode] > 0)     //Opcode has data
        {
            offset_path = ALIGN(offset_path, data_size);
        }

        if(*pcode == PATH_CLOSE)
        {
            low += step;
            hi   = low + step;
        }
        //Generate and copy the data if there's any.
        if (data_count[*pcode] > 0)
        {
            switch (format)
            {
            case VG_LITE_FP32:
                path_datafp = (float*)((uint8_t*)path_data + offset_path);
                for (j = 0; j < data_count[*pcode]; j++)
                {
                    if ((*pcode) & 0x01)
                    {
                        *path_datafp = Random_r(rel_low, rel_hi);
                    }
                    else
                    {
                        if (j % 2 == 0)
                        {
                            *path_datafp = Random_r(low, hi);
                        }
                        else
                        {
                            *path_datafp = Random_r(-draw_height / 3, draw_height / 3);
                        }
                    }
                    path_datafp++;
                    offset_path += data_size;
                }
                break;

            case VG_LITE_S32:
                path_data32 = (int*)((uint8_t*)path_data + offset_path);
                for (j = 0; j < data_count[*pcode]; j++)
                {
                    if ((*pcode) & 0x01)
                    {
                        *path_data32 = (int)Random_r(rel_low, rel_hi);
                    }
                    else
                    {
                        if (j % 2 == 0)
                        {
                            *path_data32 = (int)Random_r(low, hi);
                        }
                        else
                        {
                            *path_data32 = (int)Random_r(-draw_height / 3, draw_height / 3);
                        }
                    }
                    path_data32++;
                    offset_path += data_size;
                }
                break;

            case VG_LITE_S16:
                path_data16 = (short*)((uint8_t*)path_data + offset_path);
                for (j = 0; j < data_count[*pcode]; j++)
                {
                    if ((*pcode) & 0x01)
                    {
                        *path_data16 = (short)Random_r(rel_low, rel_hi);
                    }
                    else
                    {
                        if (j % 2 == 0)
                        {
                            *path_data16 = (short)Random_r(low, hi);
                        }
                        else
                        {
                            *path_data16 = (short)Random_r(-draw_height / 3, draw_height / 3);
                        }
                    }
                    path_data16++;
                    offset_path += data_size;
                }
                break;

            case VG_LITE_S8:
                path_data8 = (signed char*)((uint8_t*)path_data + offset_path);
                for (j = 0; j < data_count[*pcode]; j++)
                {
                    if ((*pcode) & 0x01)
                    {
                        *path_data8 = (signed char)Random_r(-128.0f, 128.0f);
                    }
                    else
                    {
                        *path_data8 = (signed char)Random_r(-64.0f, 64.0f);
                    }
                    path_data8++;
                    offset_path += data_size;
                }
                break;

            default:
                assert(FALSE);
                break;
            }
        }

        path_data8 = (int8_t*)path_data + offset_path;
        pcode++;
    }

    return offset_path;   //Return the actual size of the path data.
}

int fill_path(
              vg_lite_path_t      *path,
              int                 opcount,
              uint8_t *           opcodes,
              int                 num_subs)
{
    return fill_path_data (path->format,
        opcount,
        opcodes,
        path->path,
        num_subs);
}

void shuffle_opcodes(
                     uint8_t *opcodes,
                     int     count)
{
    int i, id;
    uint8_t temp;
    for (i = count - 2; i > 0; i--)
    {
        id = (int)Random_r(0.0f, i + 0.5f);
        temp = opcodes[i + 1];
        opcodes[i + 1] = opcodes[id];
        opcodes[id] = temp;
    }
}

vg_lite_error_t gen_path (
                          vg_lite_path_t    * path,
                          vg_lite_format_t    format,
                          int                 num_subs,
                          int                 num_segs)
{
    /**
    * 1. Gen opcode array;
    * 2. Calculate total path data size (opcode + data), create the memory;
    * 3. Fill in path memory with random path data.
    */
    uint8_t * opcodes;
    uint8_t * pdata;
    int       i, j;

    //Gen opcodes: here use single path for example.
    opcodes = (uint8_t *) MALLOC(sizeof(uint8_t) * num_segs * num_subs);

    pdata = opcodes;
    for (j = 0; j < num_subs; j++)
    {
        //Begin with a "move_to".
        *pdata++ = (uint8_t)MOVE_TO;
        //Then generate random path segments.
        for (i = 1; i < num_segs - 1; i++)
        {
            *pdata = (uint8_t)((i - 1) % (CUBI_TO_REL - LINE_TO + 1) + LINE_TO);
            pdata++;
        }
        shuffle_opcodes(opcodes + 1 + num_segs * j, num_segs - 2);
        *pdata++ = PATH_CLOSE;
    }
    //End with a path close for a sub path.
    *(pdata - 1) = PATH_DONE;

    //Calculate the data size for the opcode array.    
    path->path_length = calc_data_size(opcodes, num_segs * num_subs, format);
    path->path = MALLOC(path->path_length);
    path->format = format;
    //Generate the path.
    fill_path(path, num_segs * num_subs, opcodes, num_subs);

    FREE(opcodes);

    return VG_LITE_SUCCESS;
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: Translate
 Alpha Blending: None
 Related APIs: vg_lite_translate/vg_lite_init_path/vg_lite_clear/vg_lite_draw
 Description: Draw a single random path with high render quality.
 */
int     SFT_Path_Draw_Single()
{
    int i;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT];
    vg_lite_matrix_t matrix;
    int ret = TB_OK;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);
    vg_lite_translate(draw_width/2, draw_height/2, &matrix);
    for (i = 0; i < COUNT; i++)
    {
        // Generate path with setting arguments.
        vg_lite_init_path(&paths[i], VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, -draw_width / 2, -draw_height / 2, draw_width / 2, draw_height / 2);
        gen_path(&paths[i],
            VG_LITE_FP32,  //Type of paths data.
            1,             //Num sub paths.
            12 );          //Num path commands for each sub path.
    }

    for (i = 0; (i < COUNT) && (TB_OK == ret); i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        error = vg_lite_draw(&dst_buf,                  //dst_buf to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color);                //Color
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
            for (i = 0; i < COUNT; i++)
            {
                FREE(paths[i].path);
                vg_lite_clear_path(&paths[i]);
            }
            Exit();
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_8_", &dst_buf);       
#endif
    }

    for (i = 0; i < COUNT; i++)
    {
        FREE(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    Exit();
    
    return ret;
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: Translate
 Alpha Blending: None
 Related APIs: vg_lite_translate/vg_lite_init_path/vg_lite_clear/vg_lite_draw
 Description: Draw high render quality random paths with different coordinate types(VG_LITE_S8/VG_LITE_S16/VG_LITE_S32/VG_LITE_FP32).
 */
int SFT_Path_Draw_Formats()
{
    int i, j;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT*4];
    vg_lite_matrix_t matrix;
    int cf = 4;
    int ret = TB_OK;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);
    vg_lite_translate(draw_width/2, draw_height/2, &matrix);
    for (j = 0; j < cf; j++)
    {
        for (i = 0; i < COUNT; i++)
        {
            vg_lite_init_path(&paths[i + j * COUNT], VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, -draw_width / 2, -draw_height / 2, draw_width / 2, draw_height / 2);
            // Generate different coordinate types.
            gen_path(&paths[i + j * COUNT],
                VG_LITE_S8+j,  //Type of paths data.
                1,             //Num sub paths.
                12 );          //Num path commands for each sub path.
        }
    }

    for (i = 0; (i < COUNT * cf) && (TB_OK == ret); i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        error = vg_lite_draw(&dst_buf,                  //FB to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color);                //Color
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
            for (i = 0; i < COUNT * cf; i++)
            {
                FREE(paths[i].path);
                vg_lite_clear_path(&paths[i]);
            }
            Exit();
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_6_", &dst_buf);      
#endif
    }

    for (i = 0; i < COUNT * cf; i++)
    {
        FREE(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    Exit();
    
    return ret;
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: Translate
 Alpha Blending: None
 Related APIs: vg_lite_translate/vg_lite_init_path/vg_lite_clear/vg_lite_draw
 Description: Draw random paths with different anti-aliasing render qualities(VG_LITE_HIGH/VG_LITE_MEDIUM/VG_LITE_LOW).
 */
int     SFT_Path_Draw_Quality()
{
    int i;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    int ret = TB_OK;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_path_t paths[3];
    vg_lite_matrix_t matrix;
    vg_lite_quality_t render_quality[] = {
        VG_LITE_HIGH,
        VG_LITE_MEDIUM,
        VG_LITE_LOW
    };

    vg_lite_identity(&matrix);
    vg_lite_translate(draw_width/2, draw_height/2, &matrix);
    for (i = 0; i < 3; i++)
    {
        // Generate paths with different anti-aliasing qualities.
        vg_lite_init_path(&paths[i], VG_LITE_FP32, render_quality[i], 0, NULL, -draw_width / 2, -draw_height / 2, draw_width / 2, draw_height / 2);
        gen_path(&paths[i],
            VG_LITE_FP32,  //Type of paths data.
            1,             //Num sub paths.
            12 );          //Num path commands for each sub path.
    }

    for (i = 0; (i < 3) && (TB_OK == ret); i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        error = vg_lite_draw(&dst_buf,                  //FB to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color);                //Color
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
            for (i = 0; i < 3; i++)
            {
                FREE(paths[i].path);
                vg_lite_clear_path(&paths[i]);
            }
            Exit();
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_7_", &dst_buf);
#endif
    }

    for (i = 0; i < 3; i++)
    {
        FREE(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    Exit();
    
    return ret;
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_init_path/vg_lite_clear/vg_lite_draw
 Description: Draw high render quality paths(five-point star) with different fill rules(VG_LITE_FILL_NON_ZERO/VG_LITE_FILL_EVEN_ODD).
 */
int SFT_Path_Draw_Rule()
{
    void *path_data;    //Path data for drawing.
    uint8_t *opcode;
    uint8_t *pdata8;
    uint32_t offset;
    uint32_t path_length;
    uint8_t opfstart[] = { MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_DONE};
    vg_lite_path_t path;
    vg_lite_matrix_t matrix;
    int ret = TB_OK;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    vg_lite_identity(&matrix);

    /*
    Test path 1: A five-point star.
    */
    path_length = 4 * 5 + 1 + 8 * 5;
    path_data = MALLOC(path_length);
    opcode = (uint8_t*)path_data;
    pdata8 = (uint8_t*)path_data;
    offset = 0;

    //opcode 0: MOVE, 2 floats
    *(opcode + offset) = opfstart[0];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 50.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 100.0f;
    offset += 4;

    //opcode 1: LINE, 2 floats.
    *(opcode + offset) = opfstart[1];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 300.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -10.0f;
    offset += 4;

    //opcode 2: LINE, 2 floats.
    *(opcode + offset) = opfstart[2];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -250.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 200.0f;
    offset += 4;

    //opcode 3: LINE, 2 floats.
    *(opcode + offset) = opfstart[3];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 80.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -280.0f;
    offset += 4;

    //opcode 4: LINE, 2 floats.
    *(opcode + offset) = opfstart[4];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 80.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 260.0f;
    offset += 4;

    //opcode 5: END.
    *(opcode + offset) = opfstart[5];
    offset++;

    /* Now construct the path. */
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, draw_width, draw_height);

    /* Draw the path with 2 different rules. */
    vg_lite_clear(&dst_buf,NULL,0xffffffff);
    // Draw path with VG_LITE_FILL_NON_ZERO rule.
    error = vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_NON_ZERO, &matrix, 0, 0xffff0000);
    if (error) {
        VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
        FREE(path_data);
        path.path = NULL;
        vg_lite_clear_path(&path);
        Exit();
        return TB_BAD_API_FAIL;
    }
    vg_lite_finish();
    
#if !defined (VGLITE_TST_FIRMWARE)
    SaveBMP_SFT("gfx_5_", &dst_buf);
#endif
    
    if (TB_OK == ret) {
        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        // Draw path with VG_LITE_FILL_EVEN_ODD rule.
        error = vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000);
        if (error) {
            VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
            FREE(path_data);
            path.path = NULL;
            vg_lite_clear_path(&path);
            Exit();
            return TB_BAD_API_FAIL;
        }
        vg_lite_finish();
        
#if !defined (VGLITE_TST_FIRMWARE)
        SaveBMP_SFT("gfx_5_", &dst_buf);
#endif
    }

    /* Free data. */
    FREE(path_data);
    path.path = NULL;
    vg_lite_clear_path(&path);
    Exit();
    
    return ret;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
int SFT_Path_Draw(int tst_code, vg_lite_buffer_t * raw_fb, int user_fb_width, int user_fb_height)
{
    uint32_t ret = TB_OK;
    uint32_t feature_check = 0;
    raw = raw_fb;
    draw_width = user_fb_width;
    draw_height = user_fb_height;
    
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
        printf("gfx_sft_path_draw is not supported.\n");
        return TB_BAD;
    }
    
    InitBMP(user_fb_width, user_fb_height);
    
    if (VG_LITE_SUCCESS != Init())
    {
        VGLITE_TST_PRINTF("Draw engine initialization failed.\r\n");
    }

    if (tst_code == 1) {
        VGLITE_TST_PRINTF("\r\nCase: Path_Draw_Rule\r\n");
        ret = SFT_Path_Draw_Rule();
    } else if (tst_code == 2) {
        VGLITE_TST_PRINTF("\r\nCase: Path_Draw_Formats\r\n");
        ret = SFT_Path_Draw_Formats();
    } else if (tst_code == 3) {
        VGLITE_TST_PRINTF("\r\nCase: Path_Draw_Quality\r\n");
        ret = SFT_Path_Draw_Quality();
    } else if (tst_code == 4) {
        VGLITE_TST_PRINTF("\r\nCase: Path_Draw_Single\r\n");
        ret = SFT_Path_Draw_Single();
    } else {
        ret = TB_BAD;
    }
    
    DestroyBMP();
    vg_lite_close();
    
    return ret;
}

