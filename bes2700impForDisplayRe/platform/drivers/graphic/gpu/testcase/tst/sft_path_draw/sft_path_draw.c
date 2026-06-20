//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "SFT.h"

#define COUNT 1        //Num paths to test.

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

//-----------------------------------------------------------------------------
//Name: Init
//Parameters: None
//Returned Value: None
//Description: Create a dest buffer object
//-----------------------------------------------------------------------------
static vg_lite_error_t Init()
{
    vg_lite_error_t error;

    dst_buf.width = fb_width;
    dst_buf.height = fb_height;
    dst_buf.format = formats[4];
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
        printf("allocate dst_buf failed");
        return error;
    }
    else
    {
        printf("dst buffer initialize success");
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
    
    hi  = fb_width / 2.0f;
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
                                *path_datafp = Random_r(-fb_height / 3, fb_height / 3);
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
                                *path_data32 = (int)Random_r(-fb_height / 3, fb_height / 3);
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
                                *path_data16 = (short)Random_r(-fb_height / 3, fb_height / 3);
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

void Path_Draw_Formats()
{
    char casename[30];
    int i, j;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[COUNT*2];
    vg_lite_matrix_t matrix;
    int cf = 2;
    
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width/2, fb_height/2, &matrix);
    for (j = 0; j < cf; j++)
    {
        for (i = 0; i < COUNT; i++)
        {
            vg_lite_init_path(&paths[i + j * COUNT], 0, VG_LITE_HIGH, 0, NULL, -fb_width / 2, -fb_height / 2, fb_width / 2, fb_height / 2);
            printf("Gen Path %d\n", i + j * COUNT);
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
        
        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        vg_lite_draw(&dst_buf,                  //FB to draw into.
                     &paths[i],             //Path to draw.
                     VG_LITE_FILL_EVEN_ODD, //Filling rule.
                     &matrix,               //Matrix
                     0,                     //Blend
                     color);                //Color
        vg_lite_finish();
        SaveBMP_SFT("Path_Draw_Formats_", &dst_buf);
    }
    
    for (i = 0; i < COUNT * cf; i++)
    {
        free(paths[i].path);
        vg_lite_clear_path(&paths[i]);
    }
}

void     Path_Draw_Quality()
{
    char casename[30];
    int i;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t paths[2];
    vg_lite_matrix_t matrix;
    vg_lite_quality_t render_quality[] = {
        VG_LITE_HIGH,
        VG_LITE_LOW
    };
    
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width/2, fb_height/2, &matrix);
    for (i = 0; i < 2; i++)
    {
        vg_lite_init_path(&paths[i], 0, render_quality[i], 0, NULL, -fb_width / 2, -fb_height / 2, fb_width / 2, fb_height / 2);
        gen_path(&paths[i],
                 VG_LITE_FP32,  //Type of paths data.
                 1,             //Num sub paths.
                 12 );          //Num path commands for each sub path.
    }
    
    for (i = 0; i < 2; i++)
    {
        r = (uint32_t)Random_r(0.0f, 255.0f);
        g = (uint32_t)Random_r(0.0f, 255.0f);
        b = (uint32_t)Random_r(0.0f, 255.0f);
        a = (uint32_t)Random_r(0.0f, 255.0f);
        color = r | (g << 8) | (b << 16) | (a << 24);
        
        vg_lite_clear(&dst_buf,NULL,0xffffffff);
        printf("\ndraw path with quality: %d\n", paths[i].quality);
        vg_lite_draw(&dst_buf,                  //FB to draw into.
                     &paths[i],             //Path to draw.
                     VG_LITE_FILL_EVEN_ODD, //Filling rule.
                     &matrix,               //Matrix
                     0,                     //Blend
                     color);                //Color
        vg_lite_finish();
        SaveBMP_SFT("Path_Draw_Quality_", &dst_buf);
    }
    
    for (i = 0; i < 2; i++)
    {
        free(paths[i].path);
    }
}

void Path_Draw_Rule()
{
    void *path_data;    //Path data for drawing.
    uint8_t *opcode;
    uint8_t *pdata8;
    uint32_t offset;
    uint32_t path_length;
    uint8_t opfstart[] = { MOVE_TO, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, LINE_TO_REL, PATH_DONE};
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
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, path_length, path_data, 0.0f, 0.0f, fb_width, fb_height);
    
    /* Draw the path with 2 different rules. */
    vg_lite_clear(&dst_buf,NULL,0xffffffff);
    printf("\ndraw path with rule: %d\n", VG_LITE_FILL_NON_ZERO);
    vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_NON_ZERO, &matrix, 0, 0xffff0000);
    vg_lite_finish();
    
    SaveBMP_SFT("Path_Draw_Rule_", &dst_buf);

    vg_lite_clear(&dst_buf,NULL,0xffffffff);
    printf("\ndraw path with rule: %d\n", VG_LITE_FILL_EVEN_ODD);
    vg_lite_draw(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, 0, 0xffff0000);
    vg_lite_finish();
    SaveBMP_SFT("Path_Draw_Rule_", &dst_buf);

    /* Free data. */
    free(path_data);
    path.path = NULL;
}
/********************************************************************************
 *     \brief
 *         entry-Function
 ******************************************************************************/
void SFT_Path_Draw()
{
    if (VG_LITE_SUCCESS != Init())
    {
        printf("Draw engine initialization failed.\n");
    }
    printf("\nCase: Path_Draw:::::::::::::::::::::Started\n");
    printf("\nCase: Path_Draw_Rule:::::::::::Started\n");
    Path_Draw_Rule();
    printf("\nCase: Path_Draw_Rule:::::::::::Ended\n");
    
    printf("\nCase: Path_Draw_Formats::::::::Started\n");
    Path_Draw_Formats();
    printf("\nCase: Path_Draw_Formats::::::::Ended\n");
    
    printf("\nCase: Path_Draw_Quality::::::::Started\n");
    Path_Draw_Quality();
    printf("\nCase: Path_Draw_Quality::::::::Ended\n");
    
    Exit();
}

