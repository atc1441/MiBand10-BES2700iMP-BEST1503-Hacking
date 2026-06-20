//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

#define COUNT 10        //Num paths to test.

/* Buffers for path_draw tests. */
vg_lite_buffer_t dst_buf;

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

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
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

    dst_buf.width = (int)WINDSIZEX;
    dst_buf.height = (int)WINDSIZEY;
    dst_buf.format = formats[0];
    dst_buf.stride = 0;
    dst_buf.handle = NULL;
    dst_buf.memory = NULL;
    dst_buf.address = 0;
    dst_buf.tiled = 0;

    CHECK_ERROR(vg_lite_allocate(&dst_buf));
    printf("dst buffer initialize success");
    return VG_LITE_SUCCESS;

ErrorHandler:
    return error;
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
    uint32_t chip_id = 0;

    switch (format)
    {
    case VG_LITE_S8:    data_size = 1;  break;
    case VG_LITE_S16:   data_size = 2;  break;
    case VG_LITE_S32:
    case VG_LITE_FP32:  data_size = 4;  break;
    default:    break;
    }

    vg_lite_get_product_info(NULL, &chip_id, NULL);
    for (i = 0; i < count; i++)
    {
        /* Opcode size. */
        size++;

        if (chip_id == 0x265) {
            /* Data size. */
            if (data_size > 1)
            {
                size = ALIGN(size, data_size);
            }
        }
        else {
            /* Data size. */
            if ((data_size > 1) &&          /* Need alignment. */
                (data_count[*pdata] > 0))
            {
                size = ALIGN(size, data_size);
            }
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
    int offset_path;        /* Pointer offset in path data. */
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
    uint32_t chip_id = 0;

    hi  = WINDSIZEX / 2.0f;
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
    vg_lite_get_product_info(NULL, &chip_id, NULL);
    for (i = 0; i < opcount; i++)
    {
        *path_data8 = *pcode;       /* Copy the opcode. */
        offset_path++;
        printf("%d/%d: opcode: %d\n", i, opcount, *pcode);
        if (chip_id == 0x265) {
            if (data_size > 1)
            {
                offset_path = ALIGN(offset_path, data_size);
            }
        }
        else {
            if (data_size > 1 &&            /* Need alignment */
                data_count[*pcode] > 0)     /* Opcode has data */
            {
                offset_path = ALIGN(offset_path, data_size);
            }
        }

        if(*pcode == PATH_CLOSE)
        {
            low += step;
            hi   = low + step;
        }
        /* Generate and copy the data if there's any. */
        if (data_count[*pcode] > 0)
        {
            switch (format)
            {
            case VG_LITE_FP32:
                path_datafp = (float*)((uint8_t*)path_data + offset_path);
                printf("FP32 data: ");
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
                            *path_datafp = Random_r(-WINDSIZEY / 3, WINDSIZEY / 3);
                        }
                    }
                    printf(" %f, ", *path_datafp);
                    path_datafp++;
                    offset_path += data_size;
                }
                printf("\n");
                break;

            case VG_LITE_S32:
                path_data32 = (int*)((uint8_t*)path_data + offset_path);
                printf("S32 data: ");
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
                            *path_data32 = (int)Random_r(-WINDSIZEY / 3, WINDSIZEY / 3);
                        }
                    }
                    printf(" %d, ", *path_data32);
                    path_data32++;
                    offset_path += data_size;
                }
                printf("\n");
                break;

            case VG_LITE_S16:
                path_data16 = (short*)((uint8_t*)path_data + offset_path);
                printf("data: ");
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
                            *path_data16 = (short)Random_r(-WINDSIZEY / 3, WINDSIZEY / 3);
                        }
                    }
                    printf(" %d, ", *path_data16);
                    path_data16++;
                    offset_path += data_size;
                }
                printf("\n");
                break;

            case VG_LITE_S8:
                path_data8 = (signed char*)((uint8_t*)path_data + offset_path);
                printf("data: ");
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
                    printf(" %d, ", *path_data8);
                    path_data8++;
                    offset_path += data_size;
                }
                printf("\n");
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
    opcodes = (uint8_t *) malloc(sizeof(uint8_t) * num_segs * num_subs);

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
    path->path = malloc(path->path_length);
    path->format = format;
    //Generate the path.
    fill_path(path, num_segs * num_subs, opcodes, num_subs);

    free(opcodes);

    return VG_LITE_SUCCESS;
}

// Test for a single path drawing.
vg_lite_error_t 	SFT_Path_Draw_Single()
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT];
    vg_lite_matrix_t matrix;


    vg_lite_identity(&matrix);
    vg_lite_translate(WINDSIZEX/2, WINDSIZEY/2, &matrix);
    for (i = 0; i < COUNT; i++)
    {
        vg_lite_init_path(&paths[i], VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, -WINDSIZEX / 2, -WINDSIZEY / 2, WINDSIZEX / 2, WINDSIZEY / 2);
        printf("Gen Path %d: ----- \n", i);
        gen_path(&paths[i], 
            VG_LITE_FP32,  //Type of paths data.
            1,             //Num sub paths.
            12 );          //Num path commands for each sub path.
    }

    for (i = 0; i < COUNT; i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
        CHECK_ERROR(vg_lite_draw(&dst_buf,                  //dst_buf to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color));                //Color
        CHECK_ERROR(vg_lite_finish());

        SaveBMP_SFT("SFT_Path_Draw_Single_", &dst_buf);

        vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
    }

ErrorHandler:
    for (i = 0; i < COUNT; i++)
    {
        free(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    return error;
}

vg_lite_error_t SFT_Path_Draw_Multi()
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT];
    vg_lite_matrix_t matrix;


    vg_lite_identity(&matrix);
    vg_lite_translate(WINDSIZEX/2, WINDSIZEY/2, &matrix);
    // Generate paths with random count of sub-paths.
    for (i = 0; i < COUNT; i++)
    {
        vg_lite_init_path(&paths[i], VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, -WINDSIZEX / 2, -WINDSIZEY / 2, WINDSIZEX / 2, WINDSIZEY / 2);
        printf("Gen Path %d: ----- \n", i);
        gen_path(&paths[i],
            VG_LITE_FP32,  //Type of paths data.
            (int)Random_r(2.0f, 5.0f),             //Num sub paths.
            8 );          //Num path commands for each sub path.
    }

    for (i = 0; i < COUNT; i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
        CHECK_ERROR(vg_lite_draw(&dst_buf,                  //FB to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color));                //Color
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("SFT_Path_Draw_Multi_", &dst_buf);

        vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);
    }

ErrorHandler:
    for (i = 0; i < COUNT; i++)
    {
        free(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    return error;
}

vg_lite_error_t SFT_Path_Draw_Formats()
{
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT*4];
    vg_lite_matrix_t matrix;
    int cf = 4;


    vg_lite_identity(&matrix);
    vg_lite_translate(WINDSIZEX/2, WINDSIZEY/2, &matrix);
    for (j = 0; j < cf; j++)
    {
        for (i = 0; i < COUNT; i++)
        {
            vg_lite_init_path(&paths[i + j * COUNT], VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, -WINDSIZEX / 2, -WINDSIZEY / 2, WINDSIZEX / 2, WINDSIZEY / 2);
            printf("Gen Path %d: ----- \n", i + j * COUNT);
            gen_path(&paths[i + j * COUNT],
                VG_LITE_S8+j,  //Type of paths data.
                1,             //Num sub paths.
                12 );          //Num path commands for each sub path.
        }
    }

    for (i = 0; i < COUNT * cf; i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
        CHECK_ERROR(vg_lite_draw(&dst_buf,                  //FB to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color));                //Color
        CHECK_ERROR(vg_lite_finish());

        SaveBMP_SFT("SFT_Path_Draw_Formats_", &dst_buf);

        vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    }

ErrorHandler:
    for (i = 0; i < COUNT * cf; i++)
    {
        free(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    return error;
}

vg_lite_error_t    SFT_Path_Draw_Quality()
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[3];
    vg_lite_matrix_t matrix;
    vg_lite_quality_t render_quality[] = {
        VG_LITE_HIGH,
        VG_LITE_MEDIUM,
        VG_LITE_LOW
    };

    vg_lite_identity(&matrix);
    vg_lite_translate(WINDSIZEX/2, WINDSIZEY/2, &matrix);
    for (i = 0; i < 3; i++)
    {
        vg_lite_init_path(&paths[i], VG_LITE_FP32, render_quality[i], 0, NULL, -WINDSIZEX / 2, -WINDSIZEY / 2, WINDSIZEX / 2, WINDSIZEY / 2);
        printf("Gen Path %d: ----- \n", i);
        gen_path(&paths[i], 
            VG_LITE_FP32,  //Type of paths data.
            1,             //Num sub paths.
            12 );          //Num path commands for each sub path.
    }

    for (i = 0; i < 3; i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);

        CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
        CHECK_ERROR(vg_lite_draw(&dst_buf,                  //FB to draw into.
            &paths[i],             //Path to draw.
            VG_LITE_FILL_EVEN_ODD, //Filling rule.
            &matrix,               //Matrix
            0,                     //Blend
            color));                //Color
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("SFT_Path_Draw_Quality_", &dst_buf);

        vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    }

ErrorHandler:
    for (i = 0; i < 3; i++)
    {
        free(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
    return error;
}

vg_lite_error_t SFT_Path_Draw_Rule()
{
    vg_lite_error_t error =VG_LITE_SUCCESS;
    void *path_data;    //Path data for drawing.
    uint8_t *opcode;
    uint8_t *pdata8;
    uint32_t offset;
    uint32_t path_length;
    uint8_t opfstart[] = { MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_DONE};
    uint8_t oprects[] = {
        MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_CLOSE,
        MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_CLOSE,
        MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_DONE
    };
    uint8_t opodds[] = {
        MOVE_TO,
        LINE_TO_REL, LINE_TO_REL, LINE_TO_REL,
        LINE_TO_REL, LINE_TO_REL, LINE_TO_REL,
        LINE_TO_REL, LINE_TO_REL, LINE_TO_REL,
        LINE_TO_REL, LINE_TO_REL, LINE_TO_REL,
        PATH_DONE
    };
    vg_lite_path_t path;
    vg_lite_matrix_t matrix;

    vg_lite_identity(&matrix);

    /*
    Test path 1: A five start.
    */
    path_length = 4 * 5 + 1 + 8 * 5;
    path_data = malloc(path_length);
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
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, WINDSIZEX, WINDSIZEY);

    /* Draw the path with 2 different rules. */
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_NON_ZERO, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);
    
    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);

    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);


    /* Free data. */
    free(path_data);
    path.path = NULL;
    vg_lite_clear_path(&path);

    // Test path 2: A path composed of some individual rectangles.
    path_length = 4 * 12 + 1 + 8 * 12;
    path_data = malloc(path_length);
    opcode = (uint8_t*)path_data;
    pdata8 = (uint8_t*)path_data;
    offset = 0;

    /* Rect 1. */
    //opcode 0: MOVE, 2 floats
    *(opcode + offset) = oprects[0];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 250.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[1];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[2];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -100.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[3];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 0: CLOSE, no data
    *(opcode + offset) = oprects[4];
    offset++;

    //opcode 0: MOVE, 2 floats
    *(opcode + offset) = oprects[5];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 200.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 130.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[6];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -100.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[7];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[8];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 100.0f;
    offset += 4;
    //opcode 0: CLOSE, no data
    *(opcode + offset) = oprects[9];
    offset++;

    //opcode 0: MOVE, 2 floats
    *(opcode + offset) = oprects[10];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 90.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[11];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 120.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[12];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 140.0f;
    offset += 4;
    //opcode 0: LINE, 2 floats
    *(opcode + offset) = oprects[13];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -120.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 0: CLOSE, no data
    *(opcode + offset) = oprects[14];
    offset++;

    /* Now construct the path. */
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, WINDSIZEX, WINDSIZEY);

    /* Draw the path with 2 different rules. */
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_NON_ZERO, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);
    
    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);

    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    free(path_data);
    path.path = NULL;
    vg_lite_clear_path(&path);

    // Test path 3: Odd path with coherent edges.
    path_length = 4 * 13 + 1 + 8 * 13;
    path_data = malloc(path_length);
    opcode = (uint8_t*)path_data;
    pdata8 = (uint8_t*)path_data;
    offset = 0;

    //opcode 0: MOVE, 2 floats
    *(opcode + offset) = opodds[0];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 200.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 200.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[1];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[2];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[3];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[4];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 250.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[5];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[6];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[7];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 250.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[8];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[9];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = -150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[10];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = -250.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[11];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    //opcode 1: LINE, 2 floats
    *(opcode + offset) = opodds[12];
    offset++;
    offset = ALIGN(offset, 4);
    *(float_t*)(pdata8 + offset) = 0.0f;
    offset += 4;
    *(float_t*)(pdata8 + offset) = 150.0f;
    offset += 4;
    //opcode 1: DONE, no data.
    *(opcode + offset) = opodds[13];
    offset++;

    /* Now construct the path. */
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, WINDSIZEX, WINDSIZEY);

    /* Draw the path with 2 different rules. */
    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_NON_ZERO, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);
    
    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

    CHECK_ERROR(vg_lite_clear(&dst_buf,NULL,0xffffffff));
    CHECK_ERROR(vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("SFT_Path_Draw_Rule_", &dst_buf);
    
    vg_lite_blit(g_fb, &dst_buf, NULL, 0, 0, filter);

ErrorHandler:
    free(path_data);
    path.path = NULL;
    vg_lite_clear_path(&path);
    return error;
}
/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t SFT_Path_Draw()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    if (VG_LITE_SUCCESS != Init())
    {
        printf("Draw engine initialization failed.\n");
    }
    output_string("\nCase: Path_Draw:::::::::::::::::::::Started\n");

    output_string("\nCase: Path_Draw_Single:::::::::Started\n");
    CHECK_ERROR(SFT_Path_Draw_Single());
    output_string("\nCase: Path_Draw_Single:::::::::Ended\n");

    output_string("\nCase: Path_Draw_Multi::::::::::Started\n");
    CHECK_ERROR(SFT_Path_Draw_Multi());
    output_string("\nCase: Path_Draw_Multi::::::::::Ended\n");

    output_string("\nCase: Path_Draw_Formats::::::::Started\n");
    CHECK_ERROR(SFT_Path_Draw_Formats());
    output_string("\nCase: Path_Draw_Formats::::::::Ended\n");

    output_string("\nCase: Path_Draw_Quality::::::::Started\n");
    CHECK_ERROR(SFT_Path_Draw_Quality());
    output_string("\nCase: Path_Draw_Quality::::::::Ended\n");

    output_string("\nCase: Path_Draw_Rule:::::::::::Started\n");
    CHECK_ERROR(SFT_Path_Draw_Rule());
    output_string("\nCase: Path_Draw_Rule:::::::::::Ended\n");

ErrorHandler:
    Exit();
    return error;
}


/* ***
* Logging entry
*/
void SFT_Path_Draw_Log()
{
}

