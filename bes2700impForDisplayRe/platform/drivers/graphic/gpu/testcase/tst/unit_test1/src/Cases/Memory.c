//-----------------------------------------------------------------------------
//Description: The test cases test the path filling functions.
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

vg_lite_buffer_t buffer[5];
void clean_up()
{
    int i;
    for(i = 0;i < 5; i++) {
        if(buffer[i].handle != NULL)
            vg_lite_free(&buffer[i]);
    }
    vg_lite_close();
}
vg_lite_error_t Allocate(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    buffer->width = (int)Random_r(0, 600);
    buffer->height = (int)Random_r(0, 600);
    buffer->format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(buffer));

ErrorHandler:
    return error;
}
vg_lite_error_t Allocate_All()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    for(i = 0;i < 5; i++) {
        if(buffer[i].handle == NULL) {
            CHECK_ERROR(Allocate(&buffer[i]));
        }
    }

    return VG_LITE_SUCCESS;
ErrorHandler:
    clean_up();
    return error;
}
vg_lite_error_t Free(vg_lite_buffer_t * buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    if(buffer->handle != NULL)
        CHECK_ERROR(vg_lite_free(buffer));

ErrorHandler:
    return error;
}
vg_lite_error_t Random_Free()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;
    for(i = 0;i < 5; i++) {
        CHECK_ERROR(Free(&buffer[(int)Random_r(0, 4)]));
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    clean_up();
    return error;
}
vg_lite_error_t Memory_Test()
{
    int i,j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Memory_Test:::::::::Started\n");
    for(i = 0;i < 1000; i++) {
        CHECK_ERROR(Allocate(&buffer[0]));
        CHECK_ERROR(Free(&buffer[0]));
    }
    for(i = 0;i < 1000; i++) {
        CHECK_ERROR(Allocate_All());
        CHECK_ERROR(Random_Free());
    }
    for(i = 0;i < 1000; i++) {
        vg_lite_init(0,0);
        for(j = 0;j < 10; j++) {
            CHECK_ERROR(Allocate_All());
            CHECK_ERROR(Random_Free());
        }
        clean_up();
    }
    output_string("\nCase: Memory_Test:::::::::Ended\n");

ErrorHandler:
    clean_up();
    return error;
}
