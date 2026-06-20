/**
 * Included Files
 */


#include "semaphore.h"
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <hal_timer.h>
#include <hal_trace.h>

#define SEM_PRIO_NONE   0
#define SEM_VALUE_MAX    128
#ifndef EINVAL
#define EINVAL 1
#endif
#ifndef OK
#define OK 0
#endif
#ifndef ERROR
#define ERROR -1
#endif

/**
 * _sem_init - initializes the semaphore sem
 *
 * @ sem : Semaphore to be initialized
 * @ pshared : Process sharing (not used)
 * @ value : Semaphore initialization value
 *
 * Returns: Zero (OK) negated errno value is returned on failure.
 *
 */

int _sem_init(FAR sem_t *sem, int pshared, unsigned int value)
{
  /* Verify that a semaphore was provided and the count is within the valid
   * range.
   */

  if (sem != NULL && value <= SEM_VALUE_MAX)
    {
      memset(&sem->_osSemaphoreDef, 0, sizeof(osSemaphoreDef_t)) ; \
      sem->_osSemaphoreId = osSemaphoreCreate(&sem->_osSemaphoreDef, value);
      return OK;
    }

  return -EINVAL;
}

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

int sem_init(FAR sem_t *sem, int pshared, unsigned int value)
{
  int ret;

  ret = _sem_init(sem, pshared, value);
  if (ret < 0)
    {
      //set_errno(-ret);
      ret = ERROR;
    }

  return ret;
}

/**
 * _sem_wait - attempts to lock the semaphore referenced by 'sem'
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: Zero (OK) , A negated errno value is returned on failure.
 *
 */

int _sem_wait(FAR sem_t *sem)
{
  int ret = 0;
  ret = osSemaphoreWait(sem->_osSemaphoreId, 0xFFFFFFFF);
  return ret;
}

/**
 * sem_timedwait - attempts to lock the semaphore referenced by 'sem'
 *
 * @sem : Semaphore descriptor.
 *
 * Returns: Zero (OK) , A negated errno value is returned on failure.
 *
 */

int sem_timedwait(FAR sem_t *sem, uint32_t ms)
{
  int ret = 0;
  ret = osSemaphoreWait(sem->_osSemaphoreId, ms);
  return ret? 0 : -1;
}


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

int sem_wait_uninterruptible(FAR sem_t *sem)
{
  int ret;

  do
    {
      /* Take the semaphore (perhaps waiting) */

      ret = _sem_wait(sem);
    }
  while (ret == -EINTR);

  return ret;
}

/**
 * @sem_wait - attempts to lock the semaphore referenced by 'sem'
 *
 * @sem : Semaphore descriptor.
 *
 * Returns:
 *   zero (OK) if successful.  Otherwise, -1 (ERROR) is returned and
 *   the errno value is set appropriately.  Possible errno values include:
 */

int sem_wait(FAR sem_t *sem)
{

  return _sem_wait(sem);

}
/**
 * _sem_post - unlocks the semaphore referenced by sem
 *
 * @sem : Semaphore descriptor
 *
 * Returns:
 *   Zero (OK) is returned on success.  A negated errno value is returned on failure.
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 *
 */

int _sem_post(FAR sem_t *sem)
{
  int ret = 0;
  ret =  osSemaphoreRelease(sem->_osSemaphoreId);
  return ret;
}

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

int sem_post(FAR sem_t *sem)
{
  int ret;

  ret = _sem_post(sem);
  if (ret < 0)
    {
    //  set_errno(-ret);
      ret = ERROR;
    }

  return ret;
}


int sem_getvalue(sem_t *sem, uint32_t *currVal)
{
    int ret;

    if ((sem == NULL) || (currVal == NULL)) {
        ret = ERROR;
        return ret;
    }

    *currVal = osSemaphoreGetCount(sem);

    return ret;
}


/**
 * _sem_destroy -  destroy the semaphore indicated by 'sem'.
 *
 * @sem : Semaphore to be destroyed.
 *
 * Returns: Zero (OK) is returned on success. A negated errno value is returned on failure.
 *
 * NOTE: nothing todo, because sem memory block is allocated by the caller
 **/

int _sem_destroy (FAR sem_t *sem)
{
  /* Assure a valid semaphore is specified */

  if (sem != NULL)
    {
       osSemaphoreDelete(sem->_osSemaphoreId);
       return OK;
    }

  return -EINVAL;
}

/**
 * sem_destroy - destroy the un-named semaphore indicated by 'sem'.
 *
*
 * @sem : Semaphore to be destroyed.
 *
 * Returns: zero (OK),  -1 (ERROR)
 *
 */

int sem_destroy (FAR sem_t *sem)
{
  int ret;

  ret = _sem_destroy(sem);
  if (ret < 0)
    {
      ret = ERROR;
    }

  return ret;
}

/**
 * sem_trywait - lighter weight version of sem_timedwait()
 * @sem     : Semaphore object

 *
 * Returns: Zero (OK) is returned on success. A negated errno value is
 * returned on failure:
 *     -ETIMEDOUT is returned on the timeout condition.
 *     -ECANCELED may be returned if the thread is canceled while waiting.
 *
 */
int sem_trywait(FAR sem_t *sem)
{
  int ret = 0;
  ret = osSemaphoreWait(sem->_osSemaphoreId, 0);
  return ret;
}


/**
 * sem_set_protocol - Set semaphore protocol attribute.
 *
 *    One particularly important use of this function is when a semaphore
 *    is used for inter-task communication like:
 *
 *      TASK A                 TASK B
 *      sem_init(sem, 0, 0);
 *      sem_wait(sem);
 *                             sem_post(sem);
 *      Awakens as holder
 *
 *    In this case priority inheritance can interfere with the operation of
 *    the semaphore.  The problem is that when TASK A is restarted it is a
 *    holder of the semaphore.  However, it never calls sem_post(sem) so it
 *    becomes *permanently* a holder of the semaphore and may have its
 *    priority boosted when any other task tries to acquire the semaphore.
 *
 *    The fix is to call nxsem_set_protocol(SEM_PRIO_NONE) immediately after
 *    the sem_init() call so that there will be no priority inheritance
 *    operations on this semaphore.
 *
 * @sem      : A pointer to the semaphore whose attributes are to be
 *               modified
 * @protocol : The new protocol to use
 *
 * Returns  Zero (OK) is returned on success.  A negated errno value is returned on failure.
 *
 */

int sem_set_protocol(FAR sem_t *sem, int protocol)
{
    return 0;
}


void sem_reset(FAR sem_t *sem, unsigned int value)
{
   //first delete current sem
   osSemaphoreDelete(sem->_osSemaphoreId);
   sem->_osSemaphoreId = osSemaphoreCreate(&sem->_osSemaphoreDef, value);
}