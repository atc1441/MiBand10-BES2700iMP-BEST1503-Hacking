#include "plat_types.h"
#include "cmsis.h"
#include "hal_cmu.h"

int set_beco_coprocessor(int enable);

int beco_init(void)
{
    uint32_t lock;

    lock = int_lock();
#if defined(SCO_CP_ACCEL)
    hal_cmu_cp_beco_enable();
#else
    hal_cmu_beco_enable();
#endif
    set_beco_coprocessor(1);
    int_unlock(lock);

    return 0;
}

int beco_exit(void)
{
    uint32_t lock;

    lock = int_lock();
    set_beco_coprocessor(0);
#if defined(SCO_CP_ACCEL)
    hal_cmu_cp_beco_disable();
#else
    hal_cmu_beco_disable();
#endif
    int_unlock(lock);

    return 0;
}
