/* doom_iwad.c - bind the embedded IWAD to the engine.
 *
 * doom_wad.h (copied from the MiBand8 "middle.h") defines the DOOM1 shareware
 * IWAD as  const unsigned char rawDataA[1338700]  (verified WAD magic "IWAD").
 * w_wad.c / d_main.c expect the externs declared in doom_iwad.h. The original
 * Apollo main set these at runtime; here they are bound to the in-flash array.
 */
#include "doom_wad.h"
#include "doom_iwad.h"

const unsigned char *doom_iwad     = rawDataA;
const unsigned int   doom_iwad_len = sizeof(rawDataA);
