#ifndef __LDAC_PTHREAD_UNDEFINE_H__
#define __LDAC_PTHREAD_UNDEFINE_H__


#ifdef malloc
#undef malloc
#endif

#ifdef free
#undef free
#endif

#define pthread_mutex_destroy(...)
#define pthread_mutex_init(...)
#define pthread_mutex_lock(...)
#define pthread_mutex_unlock(...)
#define pthread_cond_signal(...)
#define pthread_cond_init(...)
#define pthread_join(...)
#define pthread_create(...)
#define pthread_cond_wait(...)
#define pthread_cond_timedwait(...)

#define pthread_t       uint32_t
#define pthread_cond_t  uint32_t
#define pthread_mutex_t uint32_t


enum ldac_BCO_decoder_BCO_data_process
{
    LDAC_BCO_DECODER_OK,
    LDAC_BCO_DECODER_CACHING,
    LDAC_BCO_DECODER_BUFFER_UNDERFLOW,
    LDAC_BCO_DECODER_FRAMES_TO_DECODE_IS_0,
    LDAC_BCO_DECODER_ERROR,
};

#endif  //__LDAC_PTHREAD_UNDEFINE_H__
