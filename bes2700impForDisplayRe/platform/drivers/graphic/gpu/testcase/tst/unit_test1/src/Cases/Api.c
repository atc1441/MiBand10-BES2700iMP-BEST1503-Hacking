#include "../SFT.h"
#include "../Common.h"

#if defined __LINUX__
#include <unistd.h>
#endif
typedef enum api {
    upload_buffer,
    get_info,
    get_product_info,
    blit_rect,
    flush
} api_t;

void Delay(uint16_t time)
{
    uint16_t i,j,k;
    for(i = 0 ; i < time; i++)
        for(j = 0; j < UINT16_MAX;j++)
            for(k = 0; k < UINT16_MAX;k++);
}
/*  test matrix operation   */
vg_lite_error_t API_Run(api_t api,uint32_t rect[])
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_buffer_t buffer,dst_buf;
    int32_t dst_width, dst_height;
    vg_lite_matrix_t matrix;
    uint8_t * data[3];
    uint32_t stride[3];
    char name[20];
    uint32_t chip_id;
    uint32_t chip_rev;
    vg_lite_info_t info;

    memset(&buffer,0,sizeof(vg_lite_buffer_t));
    memset(&dst_buf,0,sizeof(vg_lite_buffer_t));

    if(api == get_info) {
        vg_lite_get_info(&info);
        printf("api_version=%d\n",info.api_version);
        printf("header_version=%d\n",info.header_version);
        printf("release_version=%d\n",info.release_version);
        printf("reserved=%d\n",info.reserved);
    }
    if(api == get_product_info) {
        vg_lite_get_product_info(name,&chip_id,&chip_rev);
        printf("product_name=%s\n",name);
        printf("chip_id=%d\n",chip_id);
        printf("chip_rev=%d\n",chip_rev);
    }
    if(api == upload_buffer) {
        if (0 != vg_lite_load_raw_to_point(&data[0], &stride[0],"landscape.raw")) {
            printf("load raw file error\n");
            return error;
        }
        error = Allocate_Buffer(&buffer,VG_LITE_RGBA8888,320,240);
        if (error != VG_LITE_SUCCESS)
        {
            printf("[%s]allocate srcbuff %d failed. error type is %s\n", __func__, __LINE__,error_type[error]);
            return error;
        }
        CHECK_ERROR(vg_lite_buffer_upload(&buffer,data,stride));
        SaveBMP_SFT("Test_Blit_Upload_", &buffer);
    }
    if(api == blit_rect || api == flush) {
        if (0 != vg_lite_load_raw(&buffer,"landscape.raw")) {
            printf("load raw file error\n");
            return error;
        }
        dst_width = WINDSIZEX;
        dst_height = WINDSIZEY;
        CHECK_ERROR(Allocate_Buffer(&dst_buf,VG_LITE_RGBA8888,dst_width,dst_height));
        vg_lite_identity(&matrix);
        
        CHECK_ERROR(vg_lite_clear(&dst_buf, NULL, 0xffffffff));
        if(api == blit_rect) {
            error = (vg_lite_blit_rect(&dst_buf, &buffer, rect, &matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT));
            if (error != VG_LITE_SUCCESS)
            {
                if(error == VG_LITE_INVALID_ARGUMENT) {
                    printf("[%s]vg_lite_blit_rect %d invalid.error type is %s\n", __func__, __LINE__,error_type[error]);
                }else {
                    printf("[%s]vg_lite_blit_rect %d failed.error type is %s\n", __func__, __LINE__,error_type[error]);
                    Free_Buffer(&buffer);
                    Free_Buffer(&dst_buf);
                    return error;
                }
            }
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("Test_Blit_Rect_", &dst_buf);
        }else {
            CHECK_ERROR(vg_lite_blit(&dst_buf, &buffer, &matrix,VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT));
            vg_lite_flush();
            /* this is delay fuction,please set value according the platform.*/
            Delay(1);
#if defined __LINUX__
            sleep(2);
#endif
            SaveBMP_SFT("Test_Flush_", &dst_buf);
        }
        Free_Buffer(&dst_buf);
        Free_Buffer(&buffer);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(dst_buf.handle != NULL)
        Free_Buffer(&dst_buf);
    if(buffer.handle != NULL)
        Free_Buffer(&buffer);
    return error;
}
vg_lite_error_t API_Test_Upload_Buffer()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(API_Run(upload_buffer,NULL));

ErrorHandler:
    return error;
}
vg_lite_error_t API_Test_Get_Info()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(API_Run(get_info,NULL));

ErrorHandler:
    return error;
}
vg_lite_error_t API_Test_Get_Productinfo()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(API_Run(get_product_info,NULL));

ErrorHandler:
    return error;
}
vg_lite_error_t API_Test_Blit_Rect()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    uint32_t rect[4]={0};
    for(i = 0;i < 5;i++) {
        if(i == 0) {
            CHECK_ERROR(API_Run(blit_rect,rect));
        }
        else {
            rect[0] = (uint32_t)Random_i(0,WINDSIZEX*1.3);
            rect[1] = (uint32_t)Random_i(0,WINDSIZEY*1.3);
            rect[2] = (uint32_t)Random_i(0,WINDSIZEX*1.3);
            rect[3] = (uint32_t)Random_i(0,WINDSIZEY*1.3);
            CHECK_ERROR(API_Run(blit_rect,rect));
        }
    }

ErrorHandler:
    return error;
}
vg_lite_error_t API_Test_Flush()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(API_Run(flush,NULL));

ErrorHandler:
    return error;
}
vg_lite_error_t API_Test()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Test_Upload_Buffer:::::::::Started\n");
    CHECK_ERROR(API_Test_Upload_Buffer());
    output_string("\nCase: Test_Upload_Buffer:::::::::Ended\n");
    
    output_string("\nCase: Test_Get_Info:::::::::Started\n");
    CHECK_ERROR(API_Test_Get_Info());

    output_string("\nCase: Test_Get_Info:::::::::Ended\n");
    
    output_string("\nCase: Test_Get_Productinfo:::::::::Started\n");
    CHECK_ERROR(API_Test_Get_Productinfo());

    output_string("\nCase: Test_Get_Productinfo:::::::::Ended\n");

    output_string("\nCase: Test_Blit_Rect:::::::::Started\n");
    CHECK_ERROR(API_Test_Blit_Rect());
    output_string("\nCase: Test_Blit_Rect:::::::::Ended\n");

    output_string("\nCase: API_Test_Flush:::::::::Started\n");
    CHECK_ERROR(API_Test_Flush());
    output_string("\nCase: API_Test_Flush:::::::::Ended\n");

ErrorHandler:
    return error;
}

