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

el_Context gContext;

/* Initialize vg_lite related. */
static int _initialize_vglite(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    error = vg_lite_init(width, height);

    return (error == VG_LITE_SUCCESS);
}

/* Initialize elm global objects. */
static int _initialize_elm(uint32_t width, uint32_t height)
{
    int i;

    gContext.version        = VERSION;
    gContext.currentHandle  = (ELM_NULL_HANDLE + 1); /* Reserve handle 0 for error */
    gContext.objectCount    = 0;
    gContext.tessellation_width  = width;
    gContext.tessellation_height = height;
    gContext.vector_id           = -1;

    for (i = 0; i < SLOT_COUNT; i++) {
        gContext.object_slots[i] = NULL;
    }

#if (RTOS && DDRLESS) || BAREMETAL
    for (i = 0; i < sizeof(gContext.objmap_ebo) / 4; i++) {
        gContext.objmap_ebo[i] = 0;
    }
    for (i = 0; i < sizeof(gContext.objmap_evo) / 4; i++) {
        gContext.objmap_evo[i] = 0;
    }
    for (i = 0; i < sizeof(gContext.objmap_group) / 4; i++) {
        gContext.objmap_group[i] = 0;
    }
    for (i = 0; i < sizeof(gContext.objmap_grad) / 4; i++) {
        gContext.objmap_grad[i] = 0;
    }

    gContext.objcounter_grad = 0;
    gContext.objcounter_evo = 0;
    gContext.objcounter_ebo = 0;
    gContext.objcounter_group = 0;
#endif
    return 1;
}

/* Terminate vg_lite related. */
static void _terminate_vglite(void)
{
    vg_lite_close();
}

/* Terminate elm global objects. */
static void _terminate_elm(void)
{

}

/*!
 @abstract Initialize Elementary context.

 @discussion
 It should be called as the first function of Elemenatary libary, which initializes the library. Currently
 Elementary library doesn't support context concept, neigher multi-threading. Elementary library defines
 origin of coordinate system is at top-left.

 @param none

 @return none
 */
BOOL ElmInitialize(uint32_t width, uint32_t height)
{
    BOOL result = TRUE;

    do {
        result = _initialize_vglite((int32_t)width, (int32_t)height);

        if (!result)
            break;

        result = _initialize_elm(width, height);

        if (!result)
            break;
    }
    while (0);

    return result;
}

/*!
 @abstract Terminate Elementary context.

 @discussion
 This should be called when an app exits. It frees all the resource.

 @param none

 @return none
 */
void ElmTerminate(void)
{
    _terminate_elm();
    _terminate_vglite();
}
