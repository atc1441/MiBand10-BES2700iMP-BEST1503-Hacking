#include <stdlib.h>
#include "global_data.h"
#include "stdio.h"
#include "stdint.h"

#include "z_zone.h"

/* BES2700 port: the original placed this in .tcm_buffer (Apollo TCM), which the
 * best1503 linker does not map. Keep it in normal .bss (writable RAM below the
 * framebuffer). */
uint8_t dataForG[sizeof(globals_t)];
globals_t* _g = NULL;

void InitGlobals()
{
    _g = dataForG;

    memset(_g, 0, sizeof(globals_t));

    #include "global_init.h"
}
