#ifndef __vg_lite_tst_platform_h__
#define __vg_lite_tst_platform_h__

#include <stdio.h>

#define VGLITE_TST_PRINTF printf

#ifdef _WIN32
#define __func__ __FUNCTION__
#endif

#endif
