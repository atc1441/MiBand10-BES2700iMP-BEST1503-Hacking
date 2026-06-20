#include "../SFT.h"
#include "../Common.h"

#define TRANSLATE_FLAG          1
#define SCALE_FLAG              2
#define ROTATE_FLAG             4
#define PERSPECTIVE_FLAG        8

typedef enum function {
    draw,
    blit,
    pattern,
    gradient,
} function_t;

/*  test matrix operation   */
vg_lite_error_t Matrix_Operation(int32_t pathdata[],int32_t length,function_t func,int8_t operationFlag)
{
    vg_lite_buffer_t src_buf,dst_buf;
    int32_t    src_width, src_height, dst_width, dst_height;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color;
    uint8_t    r, g, b, a;
    vg_lite_path_t path;
    vg_lite_matrix_t matrix,*matrix1;
    vg_lite_float_t sx, sy, tx, ty, degrees, w0, w1;

    vg_lite_linear_gradient_t grad;
    uint32_t ramps[] = {0xff000000, 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffffff};
    uint32_t stops[] = {0, 66, 122, 200, 255};

    memset(&src_buf,0,sizeof(vg_lite_buffer_t));
    memset(&dst_buf,0,sizeof(vg_lite_buffer_t));
    if(func == pattern)
        matrix1 = (vg_lite_matrix_t*)malloc(sizeof(vg_lite_matrix_t));
    if(func == gradient) {
        memset(&grad, 0, sizeof(grad));
        if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {
            printf("Linear gradient is not supported.\n");
            vg_lite_close();
        }

        vg_lite_set_grad(&grad, 5, ramps, stops);
        vg_lite_update_grad(&grad);
        matrix1 = vg_lite_get_grad_matrix(&grad);
    }
    if(func == gradient || func == pattern || func == draw) {
        if( func == draw) {
            r = (uint8_t)Random_r(0.0f, 255.0f);
            g = (uint8_t)Random_r(0.0f, 255.0f);
            b = (uint8_t)Random_r(0.0f, 255.0f);
            a = (uint8_t)Random_r(0.0f, 255.0f);
            color = r | (g << 8) | (b << 16) | (a << 24);
        }
        memset(&path, 0, sizeof(path));
        vg_lite_init_path(&path, VG_LITE_S32, VG_LITE_HIGH, length, pathdata, -WINDSIZEX, -WINDSIZEY, WINDSIZEX, WINDSIZEY);
    }

    if(func == blit || func == pattern) {
        /* allocate src buffer */
        src_width = WINDSIZEX;
        src_height = WINDSIZEY;
        CHECK_ERROR(Allocate_Buffer(&src_buf, VG_LITE_RGBA8888, src_width, src_height));
        /* Regenerate src buffers. Check is good for transformation. */
        CHECK_ERROR(gen_buffer(0, &src_buf, VG_LITE_RGBA8888, src_buf.width, src_buf.height));
    }

    dst_width = WINDSIZEX;
    dst_height = WINDSIZEY;
    /* allocate dst buffer */
    CHECK_ERROR(Allocate_Buffer(&dst_buf, VG_LITE_BGRA8888, dst_width, dst_height));

    if(operationFlag & SCALE_FLAG) {
        sx = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
        sy = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
    }
    if(operationFlag & TRANSLATE_FLAG) {
        tx = (vg_lite_float_t)dst_buf.width / 2.0f;
        ty = (vg_lite_float_t)dst_buf.height/ 2.0f;
    }
    if(operationFlag & ROTATE_FLAG) {
        degrees = (vg_lite_float_t)Random_r(-360.0f, 360.0f);
    }

    vg_lite_identity(&matrix);
    vg_lite_translate(tx, ty, &matrix);
    vg_lite_rotate(degrees, &matrix);
    vg_lite_scale(sx, sy, &matrix);
    if(func == blit) {
        if(operationFlag & ROTATE_FLAG) {
            w0 = (vg_lite_float_t)Random_r(0.001f, 0.01f);
            w1 = (vg_lite_float_t)Random_r(0.001f, 0.01f);
        }
        vg_lite_perspective(w0, w1, &matrix);
    }

    if(func == pattern || func == gradient) {
        if(operationFlag & SCALE_FLAG) {
            sx = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
            sy = (vg_lite_float_t)Random_r(-2.0f, 2.0f);
        }
        if(operationFlag & ROTATE_FLAG) {
            degrees = (vg_lite_float_t)Random_r(-360.0f, 360.0f);
        }
        if(operationFlag & ROTATE_FLAG) {
            w0 = (vg_lite_float_t)Random_r(0.001f, 0.01f);
            w1 = (vg_lite_float_t)Random_r(0.001f, 0.01f);
        }

        vg_lite_identity(matrix1);
        vg_lite_translate(tx, ty, matrix1);
        vg_lite_rotate(degrees, matrix1);
        vg_lite_scale(sx, sy, matrix1);
        vg_lite_perspective(w0, w1, matrix1);
    }

    CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, 0xFFFFFFFF));
    switch (func)
    {
    case draw:
        vg_lite_draw(&dst_buf,&path, VG_LITE_FILL_EVEN_ODD,&matrix,VG_LITE_BLEND_NONE,color);
        if (error)
        {
            printf("[%s: %d]vg_lite_draw failed.error type is %s\n", __func__, __LINE__,error_type[error]);
            Free_Buffer(&dst_buf);
            return error;
        }
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Draw_Matrix_Operation", &dst_buf);
        break;
    case blit:
        CHECK_ERROR(vg_lite_blit(&dst_buf, &src_buf, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Blit_Matrix_Operation", &dst_buf);
        Free_Buffer(&src_buf);
        break;
    case gradient:
        vg_lite_draw_gradient(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, &grad, VG_LITE_BLEND_NONE);
        if (error)
        {
            printf("[%s: %d]vg_lite_draw_gradient failed.error type is %s\n", __func__, __LINE__,error_type[error]);
            Free_Buffer(&dst_buf);
            vg_lite_clear_grad(&grad);
            return error;
        }
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Gradient_Matrix_Operation", &dst_buf);
        vg_lite_clear_grad(&grad);
        break;
    case pattern:
        CHECK_ERROR(vg_lite_draw_pattern(&dst_buf, &path, VG_LITE_FILL_EVEN_ODD, &matrix, &src_buf, matrix1, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_COLOR, 0xffaabbcc, VG_LITE_FILTER_POINT));
        if (error)
        {
            printf("[%s: %d]vg_lite_blit failed.error type is %s\n", __func__, __LINE__,error_type[error]);
            Free_Buffer(&dst_buf);
            Free_Buffer(&src_buf);
            return error;
        }
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Pattern_Matrix_Operation", &dst_buf);
        Free_Buffer(&src_buf);
    default:
        break;
    }   
    Free_Buffer(&dst_buf);
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(src_buf.handle != NULL)
        Free_Buffer(&src_buf);
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    vg_lite_clear_grad(&grad);
    return error;
}
/*  test path matrix operation   */
vg_lite_error_t Path_Matrix_Operation()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    int flag;
    flag |= TRANSLATE_FLAG;
    flag |= ROTATE_FLAG;
    flag |= SCALE_FLAG;
    for (i = 0; i < OPERATE_COUNT; i++) {
        CHECK_ERROR(Matrix_Operation(test_path[i],length[i],draw,flag));
    }

ErrorHandler:
    return error;
}

/*  test blit matrix operation   */
vg_lite_error_t Blit_Matrix_Operation()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    int flag;
    flag |= TRANSLATE_FLAG;
    flag |= ROTATE_FLAG;
    flag |= SCALE_FLAG;
    flag |= PERSPECTIVE_FLAG;
    for (i = 0; i < OPERATE_COUNT; i++) {
        CHECK_ERROR(Matrix_Operation(NULL,NULL,blit,flag));
    }

ErrorHandler:
    return error;
}

/*  test  Gradient matrix operation   */
vg_lite_error_t Gradient_Matrix_Operation()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    int flag;
    flag |= TRANSLATE_FLAG;
    flag |= ROTATE_FLAG;
    flag |= SCALE_FLAG;
    flag |= PERSPECTIVE_FLAG;
    for (i = 0; i < OPERATE_COUNT; i++) {
        CHECK_ERROR(Matrix_Operation(test_path[i],length[i],gradient,flag));
    }

ErrorHandler:
    return error;
}

/*  test  Pattern matrix operation   */
vg_lite_error_t Pattern_Matrix_Operation()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    int flag;
    flag |= TRANSLATE_FLAG;
    flag |= ROTATE_FLAG;
    flag |= SCALE_FLAG;
    flag |= PERSPECTIVE_FLAG;
    for (i = 0; i < OPERATE_COUNT; i++) {
        CHECK_ERROR(Matrix_Operation(test_path[i],length[i],pattern,flag));
    }

ErrorHandler:
    return error;
}

vg_lite_error_t Matrix_Test()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Path_Matrix_Operation:::::::::Started\n");
    CHECK_ERROR(Path_Matrix_Operation());
    output_string("\nCase: Path_Matrix_Operation:::::::::Ended\n");

    output_string("\nCase: Blit_Matrix_Operation:::::::::Started\n");
    CHECK_ERROR(Blit_Matrix_Operation());
    output_string("\nCase: Blit_Matrix_Operation:::::::::Ended\n");

    output_string("\nCase: Gradient_Matrix_Operation:::::::::Started\n");
    CHECK_ERROR(Gradient_Matrix_Operation());
    output_string("\nCase: Gradient_Matrix_Operation:::::::::Ended\n");

    output_string("\nCase: Pattern_Matrix_Operation:::::::::Started\n");
    CHECK_ERROR(Pattern_Matrix_Operation());
    output_string("\nCase: Pattern_Matrix_Operation:::::::::Ended\n");

ErrorHandler:
    return error;
}

