#ifndef __LOCAL_SEM_H
#define __LOCAL_SEM_H

#ifdef __NuttX__
#define WITH_OS
#endif

#ifdef RTOS
#define WITH_OS
#endif

#if defined(WITH_OS)
#ifdef __NuttX__
#define lcdc_sem_post       nxsem_post
#define lcdc_sem_wait       nxsem_wait
#define lcdc_sem_init       nxsem_init
#define lcdc_sem_trywait    nxsem_trywait
#define lcdc_sem_reset      nxsem_reset
#define lcdc_sem_get_value  nxsem_get_value
#elif defined(KERNEL_RTX5)
#include "semaphore.h"
#define lcdc_sem_post       sem_post
#define lcdc_sem_wait       sem_wait
#define lcdc_sem_init       sem_init
#define lcdc_sem_trywait    sem_trywait
#define lcdc_sem_reset      sem_reset
#define lcdc_sem_get_value  sem_getvalue
#elif defined(KERNEL_LITEOS_M)
#include "graphic_conf.h"
#include "semaphore.h"
#ifdef CONFIG_FB_SYNC
#define lcdc_sem_post        sem_post
#define lcdc_sem_wait        sem_wait
#define lcdc_sem_trywait     sem_trywait
#define lcdc_sem_init        sem_init
#define lcdc_sem_reset       sem_reset
#define lcdc_sem_get_value   sem_getvalue
#else
#define lcdc_sem_post(...)      0
#define lcdc_sem_wait(...)      0
#define lcdc_sem_trywait(...)   0
#define lcdc_sem_init(...)
#define lcdc_sem_reset(...)
#define lcdc_sem_get_value(...)
#endif
#else
#error("Invalid OS SELECTED")
#endif
#else

#ifndef sem_t
#define sem_t volatile int
#endif
#ifndef FAR
#define FAR
#endif

static __inline int no_os_lcdc_sem_wait(volatile int *sem)
{
    while (1) {
        if (*sem) {
            *sem = 0;
            break;
        }
    }
    return 0;
}

static __inline int no_os_lcdc_sem_init(volatile int *sem, int flag, int initnum)
{
    if (!sem) {
        return -1;
    }

    *sem = initnum;

    return 0;
}

static __inline int no_os_lcdc_sem_reset(volatile int *sem, int count)
{
    if (!sem) {
        return -1;
    }

    *sem = count;

    return 0;
}

static __inline int no_os_lcdc_sem_trywait(volatile int *sem)
{

    if (*sem) {
        *sem = 0;
       return 0;
    }

    return -1;
}

static __inline int no_os_lcdc_sem_post(volatile int *sem)
{
    *(sem) += 1;
    return 0;
}

static __inline int no_os_lcdc_sem_get_value(volatile int *sem, FAR int *sval)
{
    if (sem && sval)
        *sval = *sem;

    return 0;
}
#define lcdc_sem_post      no_os_lcdc_sem_post
#define lcdc_sem_wait      no_os_lcdc_sem_wait
#define lcdc_sem_trywait   no_os_lcdc_sem_trywait
#define lcdc_sem_init      no_os_lcdc_sem_init
#define lcdc_sem_reset     no_os_lcdc_sem_reset
#define lcdc_sem_get_value no_os_lcdc_sem_get_value
#endif

#endif /* __LOCAL_SEM_H */
