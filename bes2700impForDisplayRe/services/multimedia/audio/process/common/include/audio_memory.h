#ifndef AUDIO_MEMORY_H
#define AUDIO_MEMORY_H

#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

void audio_heap_init(void *begin_addr, size_t size);
void *audio_malloc(size_t size);
void audio_free(void *p);
void *audio_calloc(size_t nmemb, size_t size);
void *audio_realloc(void *ptr, size_t size);
void audio_memory_info(size_t *total,
                    size_t *used,
                    size_t *max_used);

#ifdef __cplusplus
}
#endif

#endif