#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR

#include "stddef.h"
#include <stdint.h>

typedef struct
{
    void* (*malloc)(size_t);
    void* (*calloc)(size_t, size_t);
    void (*free)(void*);
    int (*log)(uint32_t attr, const char *fmt, ...);
    int (*beco_init)(void);
    int (*beco_exit)(void);
} custom_allocator;

#ifdef __cplusplus
extern "C" {
#endif

custom_allocator *default_allocator(void);

custom_allocator *audio_allocator(void);

custom_allocator *pool_allocator(void);

#ifdef __cplusplus
}
#endif

#endif
