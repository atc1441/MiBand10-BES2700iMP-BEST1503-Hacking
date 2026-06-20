#ifndef __DMA2D_SEM_H
#define __DMA2D_SEM_H

#ifdef RTOS
#define WITH_OS
#endif

#if defined(WITH_OS)
#ifdef __NuttX__
#define dma2d_sem_post       nxsem_post
#define dma2d_sem_wait       nxsem_wait
#define dma2d_sem_init       nxsem_init
#define dma2d_sem_trywait    nxsem_trywait
#define dma2d_sem_reset      nxsem_reset
#elif defined(KERNEL_RTX5)
#include "semaphore.h"
#define local_sem_post       sem_post
#define local_sem_wait       sem_wait
#define local_sem_init       sem_init
#define local_sem_trywait    sem_trywait
#define local_sem_reset      sem_reset
#define dma2d_sem_post       sem_post
#define dma2d_sem_wait       sem_wait
#define dma2d_sem_trywait    sem_trywait
#define dma2d_sem_init       sem_init
#define dma2d_sem_reset      sem_reset
#else
#error("Invalid OS SELECTED")
#endif
#else

typedef struct {
        uint32_t _sem;
} sem_t;

static __inline int no_os_dma2d_sem_wait(volatile int *sem)
{
    while (1) {
        if (*sem) {
            *sem = 0;
            break;
        }
    }
    return 0;
}

static __inline int no_os_dma2d_sem_init(volatile int *sem, int flag, int initnum)
{
    if (!sem) {
        return -1;
    }

    *sem = initnum;

    return 0;
}

static __inline int no_os_dma2d_sem_reset(volatile int *sem, int count)
{
    if (!sem) {
        return -1;
    }

    *sem = count;

    return 0;
}

static __inline int no_os_dma2d_sem_trywait(volatile int *sem)
{

    if (*sem) {
        *sem = 0;
       return 0;
    }

    return -1;
}

static __inline int no_os_dma2d_sem_post(volatile int *sem)
{
    *(sem) += 1;
    return 0;
}

#define dma2d_sem_post      no_os_dma2d_sem_post
#define dma2d_sem_wait      no_os_dma2d_sem_wait
#define dma2d_sem_trywait   no_os_dma2d_sem_trywait
#define dma2d_sem_init      no_os_dma2d_sem_init
#define dma2d_sem_reset     no_os_dma2d_sem_reset
#endif

#endif /* __DMA2D_SEM_H */
