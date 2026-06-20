#include "plat_types.h"
#include "hal_location.h"
#include "hal_trace.h"

#define GUARD_WORD                      0xdeadbeef

BOOT_DATA_LOC
uint32_t __stack_chk_guard = GUARD_WORD;

BOOT_TEXT_FLASH_LOC
void __stack_chk_init (void)
{
  __stack_chk_guard = GUARD_WORD;
}

NORETURN
void __stack_chk_fail (void)
{
  const char *msg = "*** stack smashing detected ***: terminated";
  ASSERT(0, "%s", msg);
}
