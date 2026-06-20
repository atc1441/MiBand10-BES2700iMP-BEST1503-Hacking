#ifndef __OPUS_MEMORY_H__
#define __OPUS_MEMORY_H__

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GFP_KERNEL = 0,
    GFP_ATOMIC,
    __GFP_HIGHMEM,
    __GFP_HIGH
} gfp_t;   

struct mem_element{
    unsigned int size;
    unsigned char data[0];
};

extern void rt_system_heap_init(void *begin_addr, uint32_t size);
extern void *rt_malloc(uint32_t size);
extern void *rt_realloc(void *rmem, uint32_t newsize);
extern void *rt_calloc(uint32_t count, uint32_t size);
extern void rt_free(void *rmem);

#ifdef __cplusplus
}
#endif

#endif /*__OPUS_MEMORY_H__ */
