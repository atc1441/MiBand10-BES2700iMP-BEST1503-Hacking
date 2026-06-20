#ifndef __HEAP_API__
#define __HEAP_API__
#include "stdint.h"
#include "string.h"
#include "multi_heap.h"
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__)
#define SYSPOOL_PURE __attribute__((__const__))
#else
#define SYSPOOL_PURE
#endif

void syspool_init(void);
void syspool_init_specific_size(uint32_t size);
int syspool_free_size(void);
int syspool_get_buff(uint8_t **buff, uint32_t size);
void *syspool_malloc(uint32_t size);
void *syspool_calloc(uint32_t num, uint32_t size);
int syspool_get_available(uint8_t **buff);
uint8_t* syspool_start_addr(void) SYSPOOL_PURE;
uint32_t syspool_total_size(void);
uint32_t syspool_original_size(void) SYSPOOL_PURE;

#if defined(A2DP_LDAC_ON)
int syspool_force_used_size(uint32_t size);
#endif

void cp_pool_init(void);
int cp_pool_free_size(void);
int cp_pool_get_buff(uint8_t **buff, uint32_t size);
int cp_pool_get_available(uint8_t **buff);
uint8_t* cp_pool_start_addr(void) SYSPOOL_PURE;
uint32_t cp_pool_total_size(void);

#if defined(A2DP_DECODER_CROSS_CORE) || defined(GAF_CODEC_CROSS_CORE) || defined(APP_MCPP_SRV)
typedef void (*off_bth_user_callback)(void);
typedef enum {
    SYSPOOL_USER_SPEECH = 0,
    SYSPOOL_USER_A2DP,
    SYSPOOL_USER_GAF_ENCODE,
    SYSPOOL_USER_GAF_DECODE,
    SYSPOOL_USER_MAX,
} off_bth_syspool_user;

int off_bth_syspool_get_buff(uint8_t **buff, uint32_t size);
void *off_bth_syspool_malloc(uint32_t size);
void *off_bth_syspool_calloc(uint32_t num, uint32_t size);
int off_bth_syspool_free_size(void);
void off_bth_syspool_init(off_bth_syspool_user user, off_bth_user_callback cb);
void off_bth_syspool_deinit(off_bth_syspool_user user);
#endif

#ifdef BLE_BIS_TRANSPORT
void ble_bis_tran_syspool_init(void);
uint32_t ble_bis_tran_syspool_original_size(void);
uint32_t ble_bis_tran_syspool_get_free_size(void);
uint32_t ble_bis_tran_syspool_get_buff(uint8_t **buff, uint32_t size);
#endif

#define heap_malloc multi_heap_malloc

#define heap_check_size_malloc_available multi_heap_check_size_malloc_available

#define heap_free multi_heap_free

#define heap_realloc multi_heap_realloc

#define heap_get_allocated_size multi_heap_get_allocated_size

#define heap_register multi_heap_register

#define heap_free_size multi_heap_free_size

#define heap_minimum_free_size multi_heap_minimum_free_size

#define heap_get_info multi_heap_get_info

#define heap_dump multi_heap_dump

#define heap_dump_with_callback multi_heap_dump_with_callback

#define heap_check multi_heap_check

#define heap_ptr_in_heap multi_heap_ptr_in_heap

void *heap_malloc_align(multi_heap_handle_t heap, size_t size, size_t align);

void heap_free_align(multi_heap_handle_t heap, void *p);

#define heap_enable_tail_guard multi_heap_enable_tail_guard
#define heap_disable_tail_guard multi_heap_disable_tail_guard

typedef struct multi_heap_info *heap_handle_t;

void heap_memory_info(heap_handle_t heap,
                    size_t *total,
                    size_t *used,
                    size_t *max_used);

#if 0
void *malloc(size_t size);
void free(void *p);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
#endif
void med_heap_init(void *begin_addr, size_t size);
void med_heap_set_cp(int switch_cp);
void med_heap_add_block(void *begin_addr, size_t size);
void *med_malloc(size_t size);
void med_free(void *p);
void med_heap_reset(void);
void *med_calloc(size_t nmemb, size_t size);
void *med_realloc(void *ptr, size_t size);
void med_memory_info(size_t *total,
                    size_t *used,
                    size_t *max_used);

#ifdef __cplusplus
}
#endif
#endif
