//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

/*  unit test   */

vg_lite_error_t Pattern_Draw(void * pathdata,int32_t length,vg_lite_format_t format,vg_lite_quality_t qualities,vg_lite_filter_t filter_mode,vg_lite_blend_t blend_mode)
{
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int32_t    src_width, src_height, dst_width, dst_height;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xffa0a0a0;
    vg_lite_path_t path;
    vg_lite_matrix_t matrix;

    vg_lite_identity(&matrix);
    memset(&path, 0, sizeof(path));
    memset(&src_buf,0,sizeof(vg_lite_buffer_t));
    memset(&dst_buf,0,sizeof(vg_lite_buffer_t));
    if(format == VG_LITE_FP32)
        build_paths(&path, qualities, filter_mode,blend_mode);
    else
        vg_lite_init_path(&path, format, qualities, length, pathdata, -WINDSIZEX, -WINDSIZEY, WINDSIZEX, WINDSIZEY);
    /* generate src buffer */
    src_width = WINDSIZEX;
    src_height = WINDSIZEY;
    dst_width = WINDSIZEX;
    dst_height = WINDSIZEY;
    CHECK_ERROR(Allocate_Buffer(&src_buf, VG_LITE_BGRA8888, src_width, src_height));
    CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_BGRA8888, src_buf.width, src_buf.height));
    /* allocate dst buffer */
    CHECK_ERROR(Allocate_Buffer(&dst_buf, VG_LITE_BGRA8888, dst_width, dst_height));
    /* draw pattern */
    CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, color));
    CHECK_ERROR(vg_lite_draw_pattern(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, &src_buf, &matrix, blend_mode, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter_mode));
    CHECK_ERROR(vg_lite_finish());
    SaveBMP_SFT("Pattern_Draw_", &dst_buf);

ErrorHandler:
    /* free buffer */
    if(path.path != NULL) {
        if(format == VG_LITE_FP32) {
            free(path.path);
            path.path = NULL;
        }
    }
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    if(src_buf.handle != NULL)
        Free_Buffer(&src_buf);
    return error;
}
/*  test pattern formats   */
vg_lite_error_t Pattern_Draw_Test()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i,j,k,m;

    for (i = 0; i < FORMATS_COUNT; i++)
            for (j = 0; vg_lite_query_feature(gcFEATURE_BIT_VG_QUALITY_8X) ? j < QUALITY_COUNT : j < QUALITY_COUNT - 1; j++)
                for (k = 0; k < FILTER_COUNT; k++)
                    for (m = 0; m < BLEND_COUNT; m++)
                    {
                        if(formats[i] == VG_LITE_S8) {
                            CHECK_ERROR(Pattern_Draw(path_data10,19,formats[i],qualities[j],filter_modes[k],blend_mode[m]));
                        }
                        else if(formats[i] == VG_LITE_S16) {
                            CHECK_ERROR(Pattern_Draw(path_data11,2*19,formats[i],qualities[j],filter_modes[k],blend_mode[m]));
                        }
                        else {
                            CHECK_ERROR(Pattern_Draw(test_path[m],length[m],formats[i],qualities[j],filter_modes[k],blend_mode[m]));
                        }
                    }
ErrorHandler:
    return error;
}
/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Pattern_Test()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Pattern_Draw_test:::::::::Started\n");
    CHECK_ERROR(Pattern_Draw_Test());
    output_string("\nCase: Pattern_Draw_test:::::::::Ended\n");

ErrorHandler:
    return error;
}
