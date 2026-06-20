/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/

#include "elm_precom.h"
#include "vg_lite.h"
#include "hal_trace.h"
#include "gpu_port.h"

#undef malloc
#define malloc gpu_malloc

#undef  calloc
#define calloc(num_objects,object_size) gpu_malloc(num_objects * object_size)

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#define printf  GPU_REP

#if !DDRLESS
static vg_lite_buffer_format_t _buffer_format_to_vglite(ELM_BUFFER_FORMAT format)
{
    vg_lite_buffer_format_t fmt;
    switch (format) {
        case ELM_BUFFER_FORMAT_RGBA8888:
            fmt = VG_LITE_RGBA8888;
            break;

        case ELM_BUFFER_FORMAT_RGBX8888:
            fmt = VG_LITE_RGBX8888;
            break;

        case ELM_BUFFER_FORMAT_BGRA8888:
            fmt = VG_LITE_BGRA8888;
            break;

        case ELM_BUFFER_FORMAT_BGRX8888:
            fmt = VG_LITE_BGRX8888;
            break;

        case ELM_BUFFER_FORMAT_RGB565:
            fmt = VG_LITE_RGB565;
            break;

        case ELM_BUFFER_FORMAT_BGR565:
            fmt = VG_LITE_BGR565;
            break;

        case ELM_BUFFER_FORMAT_RGBA4444:
            fmt = VG_LITE_RGBA4444;
            break;

        case ELM_BUFFER_FORMAT_BGRA4444:
            fmt = VG_LITE_BGRA4444;
            break;

        default:
            fmt = VG_LITE_RGBA8888;
            break;
    }

    return fmt;
}
#endif
/*!
 @abstract Create internal render buffer.

 @discussion
 This functiois is to create an internal render buffer for Elementary rendering, ussually it's not for direct display.
 The buffer which is displayed on pannel is wrapped up by another API, whose address is managed by display controller side.

 @param width
 The buffer's width.

 @param height
 The buffer's height.

 @param format
 The buffer's format, check enumeration of ELM_BUFFER_FORMAT.

 @return
 The buffer handle.
 */
ElmBuffer ElmCreateBuffer(unsigned int width, unsigned int height, ELM_BUFFER_FORMAT format)
{
#if !DDRLESS
    el_Obj_Buffer *buffer_obj;
    vg_lite_buffer_t *buffer;
    vg_lite_error_t error;
    ElmHandle handle = ELM_NULL_HANDLE;

    do {
        /* Allocate ebo object. */
        buffer_obj = (el_Obj_Buffer *)calloc(1,sizeof(el_Obj_Buffer));
        if (buffer_obj != NULL) {
            buffer_obj->object.type = ELM_OBJECT_TYPE_BUF;

            // Allocate the buffer.
            buffer = &buffer_obj->buffer;
            memset(buffer, 0, sizeof(vg_lite_buffer_t));
            buffer->width  = width;
            buffer->height = height;
            buffer->format = (vg_lite_buffer_format_t)format;
            error = vg_lite_allocate(buffer);
            if (error)
                goto error_exit;

            printf("%s fb:0x%p s:%d", __func__, buffer->memory, buffer->stride * buffer->height);
            JUMP_IF_NON_ZERO_VALUE(add_object((el_Object *)buffer_obj), error_exit);
            handle = buffer_obj->object.handle;
        }
    } while(0);

    return handle;
error_exit:
    vg_lite_free(buffer);
    free(buffer_obj);
    return ELM_NULL_HANDLE;
#else
    return ELM_NULL_HANDLE;
#endif
}

/*!
 @abstract Wrap a customized buffer.

 @discussion
 The application may wrap a user created buffer by giving the information of
 the buffer including the size, the memory addresses and format. E.g., the
 application can wrap a system framebuffer thus ELM can directly render onto
 the screen.

 @return
 The buffer handle.
 */
ElmBuffer ElmWrapBuffer(int width, int height, int stride,
                        void *logical, uint32_t physical,
                        ELM_BUFFER_FORMAT format)
{
#if !DDRLESS
    el_Obj_Buffer *buffer_obj;
    vg_lite_buffer_t *buffer;
    ElmHandle handle = ELM_NULL_HANDLE;

    do {
        /* open framebuffer. */
        buffer_obj = (el_Obj_Buffer *)calloc(1,sizeof(el_Obj_Buffer));
        if (buffer_obj != NULL) {
            buffer_obj->object.type = ELM_OBJECT_TYPE_BUF;
            buffer = &buffer_obj->buffer;

            buffer->width = width;
            buffer->height = height;
            buffer->stride = stride;
            buffer->memory = logical;
            buffer->handle = NULL;
            buffer->address = physical;
            buffer->format = _buffer_format_to_vglite(format);
            buffer->tiled  = VG_LITE_LINEAR;

            JUMP_IF_NON_ZERO_VALUE(add_object((el_Object *)buffer_obj), error_exit);
            handle = buffer_obj->object.handle;
        }
    } while(0);

    return handle;

error_exit:
    free(buffer_obj);
    return ELM_NULL_HANDLE;
#else
    return ELM_NULL_HANDLE;
#endif

}

/*!
 @abstract Get buffer address.

@discussion
The function is to get the address of ElmBuffer.

@return
The buffer address.
*/

uint32_t ElmGetBufferAddress(ElmBuffer buffer)
{
#if !DDRLESS
    el_Obj_Buffer *buff_obj;
    buff_obj = (el_Obj_Buffer *)get_object(buffer);

    if (buff_obj == NULL)
    {
        return 0;
    }
    else
    {
        return buff_obj->buffer.address;
    }
#else
    return 0;
#endif
}

/*!
 @abstract Destroy a render buffer.

 @discussion
 This function is to release all internal resource inside Elementary libary belonging to this buffer.
 Applicatoin need make sure the buffer is not being used by elmentary library any more when calling this function.

 @param buffer
 The render buffer handle

 @return
 If destroying is completed successfully.
 */
BOOL ElmDestroyBuffer(ElmBuffer buffer)
{
#if !DDRLESS
    /* Find the object. */
    el_Obj_Buffer *buff = (el_Obj_Buffer *)get_object(buffer);

    /* If found, delete the vg_lite_buffer object. Otherwise, return FALSE. */
    if (buff != NULL) {
        if (buff->buffer.handle != NULL) {
            // Free the buffer memory.
            vg_lite_free(&buff->buffer);
        }

        remove_object((el_Object*)buff);
        free(buff);

        return TRUE;
    }
    else {
        return FALSE;
    }
#else
    return TRUE;
#endif
}

BOOL ElmSaveBuffer(ElmBuffer buffer, const char *name)
{
#if !DDRLESS
    el_Obj_Buffer *buff = (el_Obj_Buffer *)get_object(buffer);

    /* If found, delete the vg_lite_buffer object. Otherwise, return FALSE. */
    if (buff != NULL) {
#if !RTOS
        /*
         * "vg_lite_save_png" function does not exist (anymore). Probably a left
         * over from an older driver.
         */
         if (buff->buffer.memory != NULL) {
             vg_lite_save_png(name, &buff->buffer);
         }
#endif
        return TRUE;
    }
    else {
        return FALSE;
    }
#else
    return TRUE;
#endif
}
