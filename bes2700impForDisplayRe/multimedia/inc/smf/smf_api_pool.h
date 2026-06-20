#pragma once
#ifndef __SMF_API_POOL_H__
#define __SMF_API_POOL_H__
#include "smf_common.h"

EXTERNC void smf_std_pool_register(void);

/** print memory pool information.
 */
EXTERNC void smf_pool_print(void);

/** clean pools statistics information.
 */
EXTERNC void smf_pool_clean_statistics(void);

/** create a pool with callback for Base, otherwise the object is alloc memory from global operator new().
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_globle_pool_with_callback(void*(*cbAlloc)(uint32_t size),void(*cbFree)(void*ptr));

/** create a pool with callback for encoder, all encoders alloc memory from this pool.
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_encoder_pool_with_callback(void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr));

/** create a pool with callback for decoder, all decoders alloc memory from this pool.
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_decoder_pool_with_callback(void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr));

/** create a pool with callback for muxer, all muxers alloc memory from this pool.
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_muxer_pool_with_callback(void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr));

/** create a pool with callback for demuxer, all demuxers alloc memory from this pool.
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_demuxer_pool_with_callback(void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr));

/** create a pool with buffer for Base, otherwise the object is alloc memory from global operator new().
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return IPool pointer
 */
EXTERNC void* smf_register_globle_pool_with_buffer(void*buff,int size);

/** create a pool with buffer && threshhold for Base, otherwise the object is alloc memory from global operator new().
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @param thr[in]: the max allowed size per alloc
 *  @return IPool pointer
 */
EXTERNC void* smf_register_globle_pool_with_buffer_and_thr(void* buff, int size, int thr);

/** create a pool with buffer for encoder, all encoders alloc memory from this pool.
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return IPool pointer
 */
EXTERNC void* smf_register_encoder_pool_with_buffer(void*buff,int size);

/** create a pool with buffer for decoder, all decoders alloc memory from this pool.
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return IPool pointer
 */
EXTERNC void* smf_register_decoder_pool_with_buffer(void*buff,int size);

/** create a pool with buffer for muxer, all muxers alloc memory from this pool.
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return IPool pointer
 */
EXTERNC void* smf_register_muxer_pool_with_buffer(void*buff,int size);

/** create a pool with buffer for demuxer, all demuxers alloc memory from this pool.
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return IPool pointer
 */
EXTERNC void* smf_register_demuxer_pool_with_buffer(void*buff,int size);

/** register memory pool for object
 *  @param hd[in]: the instance handle of object 
 *  @param cbAlloc[in]: the alloc callback
 *  @param cbFree[in]: the free callback
 *  @return IPool pointer
 */
EXTERNC void* smf_register_pool_with_callback(void* hd, void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr));

/** register memory pool for object
 *  @param hd[in]: the instance handle of object
 *  @param buff[in]: the pool buffer
 *  @param size[in]: the pool buffer size
 *  @return true/false
 */
EXTERNC void* smf_register_pool_with_buffer(void* hd, void* buff, int size);

/** alloc memory from pool in hd.
 * @param hd[in]: the instance handle of object
 * @param size[in]: request size
 * @return memory pointer.
 */
EXTERNC void* smf_alloc(void* hd, int size);

/** alloc open_param from pool in hd.
 * @param hd[in]: the instance handle of object
 * @return the pointer of open_param.
 */
EXTERNC void* smf_alloc_open_param(void* hd);

/** free buff to pool in hd
 * @param hd[in]: the instance handle of object
 * @param buff[in]: free pointer
 */
EXTERNC void smf_free(void* hd, void* buff);

/** get the pool of object.
 * @param hd[in]: the instance handle of object
 * @return the pool.
 */
EXTERNC void* smf_get_pool(void* hd);

/** set the pool of object.
 * @param hd[in]: the instance handle of object
 * @param pool[in]: the IPool pointer
 * @return true/false.
 */
EXTERNC bool smf_set_pool(void* hd,void* pool);
/**/
typedef struct {
	int used;
	int freed;
	int total;
	int used_max;
}smf_pool_info_t;
/** get information of pool
 * @param pool[in]:pool handle
 * @param info[out]:information pointer
 * @return true/false
 */
EXTERNC bool smf_pool_get_info(void*pool,smf_pool_info_t*info);

/** set limit size for pool
 * @param pool[in]:pool handle
 * @param max_used_size[in]:limit max used size
 * @return true/false
 */
EXTERNC bool smf_pool_set_limit_used(void*pool,int max_used_size);

/** alloc shared memory for fifo
 * @param item_size[in]: fifo item size
 * @param item_num[in]: fifo item number
 * @param pool[in]: alloc from pool, if equal 0, alloc memory from globle pool.
 * @return fifo pointer
 */
EXTERNC smf_fifo_t* smf_alloc_shared_fifo(int item_size, int item_num, void* pool);

/** alloc shared memory for shared pool
 * @param size[in]: shared pool size
 * @param pool[in]: alloc from pool, if equal 0, alloc memory from globle pool.
 * @return shared pool pointer
 */
EXTERNC smf_shared_pool_t* smf_alloc_shared_pool(int size, void* pool);

/**/
EXTERNC void* smf_register_pool(const char* name, void* buff, unsigned size, unsigned thr);

/**/
EXTERNC void* smf_register_pool_text(const char* name, void* buff, unsigned size);

/**/
EXTERNC void* smf_register_pool_callback(const char* name
	, void* (*cbAlloc)(uint32_t size), void(*cbFree)(void* ptr)
	, uint32_t(*cbGetSize)(), uint32_t(*cbGetTotal)()
);

/**/
EXTERNC void smf_register_globle_pool(void* buff, int size);

/**/
EXTERNC void smf_designate_globle_pool(const char* name);

/**/
EXTERNC void smf_register_backup_pool(const char* name, const char* bkpool);

#endif
