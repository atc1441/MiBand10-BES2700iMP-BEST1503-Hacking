//-----------------------------------------------------------------------------
//Description: The test cases test the blitting of different size/formats/transformations of image.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

vg_lite_error_t Gradient_Draw(int32_t pathdata[],int32_t length,vg_lite_blend_t blend_mode,uint32_t ramps[],uint32_t stops[],uint8_t count)
{
    vg_lite_buffer_t dst_buf;
    int32_t dst_width, dst_height;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xffa0a0a0;
    vg_lite_path_t path;

    vg_lite_matrix_t matPath;
    vg_lite_matrix_t *matGrad;
    vg_lite_linear_gradient_t grad;

    memset(&grad, 0, sizeof(grad));
    memset(&dst_buf,0,sizeof(vg_lite_buffer_t));
    if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {
        printf("Linear gradient is not supported.\n");
        vg_lite_close();
    }

    vg_lite_set_grad(&grad,count, ramps, stops);
    vg_lite_update_grad(&grad);
    matGrad = vg_lite_get_grad_matrix(&grad);

    memset(&path, 0, sizeof(path));
    vg_lite_init_path(&path, VG_LITE_S32, VG_LITE_HIGH, length, pathdata, -WINDSIZEX, -WINDSIZEY, WINDSIZEX, WINDSIZEY);
    dst_width = WINDSIZEX;
    dst_height = WINDSIZEY;
    /* allocate dst buffer */
    CHECK_ERROR(Allocate_Buffer(&dst_buf, VG_LITE_BGRA8888, dst_width, dst_height));

    vg_lite_identity(matGrad);
    vg_lite_rotate(30.0f, matGrad);
    vg_lite_identity(&matPath);

    /* draw gradient */
    CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, color));
    vg_lite_draw_gradient(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &grad, blend_mode);
    CHECK_ERROR(vg_lite_finish());

    SaveBMP_SFT("Gradient_Draw_Blend_modes_", &dst_buf);

ErrorHandler:
    /* free buffer */
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    vg_lite_clear_grad(&grad);
    return error;
}
vg_lite_error_t Gradient_Draw_Blendmodes()
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t ramps[] = {0xff000000, 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffffff};
    uint32_t stops[] = {0, 66, 122, 200, 255};
    for (i = 0; i < BLEND_COUNT; i++) {
        CHECK_ERROR(Gradient_Draw(test_path[i],length[i],blend_mode[i],ramps,stops,5));
    }

ErrorHandler:
    return error;
}
vg_lite_error_t Gradient_Draw_ColorCount()
{
    int i,j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    uint32_t ramps[COLOR_COUNT + 1] = {0};
    uint32_t stops[COLOR_COUNT + 1] = {0};
    int color_count[COLOR_COUNT + 2];

    for(i = 0; i <= COLOR_COUNT + 1; i++) {
        /*  equal 0 or out-of-range*/
        color_count[i] = i;
        for(j = 0; j < color_count[i]; j++) {
            /*  out-older   */
            if(color_count[i] % 4 == 0) {
                ramps[j] = (uint32_t)Random_i(0xFF000000,0xFFFFFFFF);
                stops[j] = (uint32_t)Random_i(0,300);
            }
            else {
                /*  normal  */
                ramps[j] = (uint32_t)Random_i(0xFF000000,0xFFFFFFFF);
                if(j > 0) {
                    if(stops[j - 1] == 255)
                        stops[j] = 255;
                    else
                        stops[j] = (uint32_t)Random_i(stops[j-1],255);
                }
                else {
                    stops[j] = (uint32_t)Random_i(0,255);
                }
            }
        }
        CHECK_ERROR(Gradient_Draw(test_path[i%9],length[i%9],VG_LITE_BLEND_NONE,ramps,stops,color_count[i]));
    }

ErrorHandler:
    return error;
}
/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Gradient_Test()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Gradient_Draw_Blendmodes:::::::::Started\n");
    CHECK_ERROR(Gradient_Draw_Blendmodes());
    output_string("\nCase: Gradient_Draw_Blendmodes:::::::::Ended\n");

    output_string("\nCase: Gradient_Draw_ColorCount:::::::::Started\n");
    CHECK_ERROR(Gradient_Draw_ColorCount());
    output_string("\nCase: Gradient_Draw_ColorCount:::::::::Ended\n");

ErrorHandler:
    return error;
}

