#ifndef __INCLUDE_GPU_SEMAPHORE_H
#define __INCLUDE_GPU_SEMAPHORE_H
#include "cmsis_os.h"
#if defined(KERNEL_RTX5)
#include "rtx5/rtx_lib.h"
#endif

#define FAR


typedef struct {
    osSemaphoreId    _osSemaphoreId;
    osSemaphoreDef_t _osSemaphoreDef;
#if defined(KERNEL_RTX5)
    char name[32];
    os_semaphore_t _semaphore_data;
#elif defined(KERNEL_RTX)
    uint32_t _semaphore_data[2];
#endif
} sem_t;


#if defined(KERNEL_RTX5)
     #define SEM_INITIALIZER(p,c) \
        *p; \
        p->_osSemaphoreDef.name = p->name; \
        p->_osSemaphoreDef.cb_mem = &p->_semaphore_data; \
        p->_osSemaphoreDef.cb_size = sizeof(os_semaphore_t); \
        p->_osSemaphoreId = osSemaphoreCreate(&p->_osSemaphoreDef, c);

#elif defined(KERNEL_RTX)

     #define SEM_INITIALIZER(p,c) \
        *p; \
        p->_osSemaphoreDef.semaphore = p->_semaphore_data; \
        p->_osSemaphoreId = osSemaphoreCreate(&p->_osSemaphoreDef, c);

#endif

/**
 * sem_init - initializes the semaphore sem
 *
 * @sem : Semaphore to be initialized
 * @pshared : Process sharing (not used)
 * @value : Semaphore initialization value
 *
 * Returns:zero (OK) if successful.  Otherwise, -1 (ERROR)
 *
 */

int sem_init(FAR sem_t *sem, int pshared, unsigned int value);


/**
 * sem_wait_uninterruptible - wrapped version of nxsem_wait(), which is
 *   uninterruptible and convenient for use.
 *
 * @sem : Semaphore descriptor.
 *
 * Returns:
 *   Zero(OK)  : On success
 *   EINVAL    : Invalid attempt to get the semaphore
 *   ECANCELED : May be returned if the thread is canceled while waiting.
 *
 */

int sem_wait_uninterruptible(FAR sem_t *sem);

/**
 * @sem_wait - attempts to lock the semaphore referenced by 'sem'
 *
 * @sem : Semaphore descriptor.
 *
 * Returns:
 *   zero (OK) if successful.  Otherwise, -1 (ERROR) is returned and
 *   the errno value is set appropriately.  Possible errno values include:
 */

int sem_wait(FAR sem_t *sem);

/**
 * sem_post - unlocks the semaphore referenced by sem
 *
 * @sem : Semaphore descriptor
 *
 * Returns: zero (OK) if successful.  Otherwise, -1 (ERROR)
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 *
 */

int sem_post(FAR sem_t *sem);

/**
 * sem_destroy - destroy the un-named semaphore indicated by 'sem'.
 *
*
 * @sem : Semaphore to be destroyed.
 *
 * Returns: zero (OK),  -1 (ERROR)
 *
 */

int sem_destroy (FAR sem_t *sem);


/**
 * sem_trywait - lighter weight version of sem_timedwait()
 * @sem     : Semaphore object

 * Returns: Zero (OK) is returned on success. A negated errno value is
 * returned on failure:
 *     -ETIMEDOUT is returned on the timeout condition.
 *     -ECANCELED may be returned if the thread is canceled while waiting.
 *
 */
int sem_trywait(FAR sem_t *sem);


void sem_reset(FAR sem_t *sem, unsigned int value);


/**
 * sem_timedwait - attempts to lock the semaphore referenced by 'sem'
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: Zero (OK) , A negated errno value is returned on failure.
 *
 */

int sem_timedwait(FAR sem_t *sem, uint32_t ms);

int sem_getvalue(sem_t *sem, uint32_t *currVal);

#endif //__INCLUDE_GPU_SEMAPHORE_H

