#include <stdio.h>

#include "vg_lite.h"
#include "vg_lite_util.h"

#include "util.h"
#include "Common.h"
#include "SFT.h"


float   WINDSIZEX = 320.f;
float   WINDSIZEY = 240.f;


/* initialization */
int init()
{
    vg_lite_error_t error;
    CHECK_ERROR(vg_lite_init(384, 240));
    if (error != VG_LITE_SUCCESS)
    {
        printf("vg_lite engine initialization failed.\n");
    }
    else
    {
        printf("vg_lite engine initialization OK.\n");
    }

ErrorHandler:
    return error;
}

/* destroy VGLite resources */
void destroy()
{
    vg_lite_close();
}

int main (int argc, char** argv)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int ret;

    /* initialize */
    ret = init();
    if (ret != 0)
        return -1;

    /* Draw */
    CHECK_ERROR(Render());
    /* destroy */
    destroy();

ErrorHandler:
    return;
}


