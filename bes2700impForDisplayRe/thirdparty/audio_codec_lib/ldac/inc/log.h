/*******************************************************************************
    Copyright 2016-2017 Sony Corporation
*******************************************************************************/
#ifndef LOG_H__
#define LOG_H__

#if defined __cplusplus
extern "C" {
#endif /* __cplusplus */

//#define BCO_TRACE_OPEN 1
#ifndef BCO_TRACE_OPEN
#define INFO(fmt, ...)
#define DBG(fmt, ... )
#define DBG_NOINFO(fmt, ... )
#define ERR(fmt, ...)
#else
#define INFO(str, ...)       TRACE(2, str, ##__VA_ARGS__)
#define DBG(str, ...)        TRACE(2, str, ##__VA_ARGS__)
#define DBG_NOINFO(str, ...) TRACE(2, str, ##__VA_ARGS__)
#define ERR(str, ...)        TRACE(2, str, ##__VA_ARGS__)
#endif

#include "string.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "hal_trace.h"
#include "hal_timer.h"
#include "cmsis.h"
#include "stdlib.h"
#include "ldac_pthread_undefine.h"


struct time_ldac_spec{
    uint32_t tv_sec;    //time_t tv_sec	整秒数（合法值 >= 0）
    uint32_t tv_nsec;   //long tv_nsec  纳秒数（合法值为 [0, 999999999] ）
};

#define malloc(p)  ldac_bco_malloc(p);
#define free(s)    ldac_bco_free(s)
//#include <stdint.h>
extern int syspool_get_buff(uint8_t **buff, uint32_t size);
static POSSIBLY_UNUSED void *ldac_bco_malloc(size_t size)
{
	void *ptr = NULL;

	INFO("[%s]", __FUNCTION__);
	syspool_get_buff((uint8_t **)&ptr, size);

	return ptr;
}

static POSSIBLY_UNUSED void *ldac_bco_calloc(size_t nmemb, size_t size)
{
	void *ptr = ldac_bco_malloc(nmemb * size);
	if (ptr != NULL)
		memset(ptr, 0, nmemb * size);
	return ptr;
}

static POSSIBLY_UNUSED void ldac_bco_free(void *ptr)
{
    INFO("[%s]", __FUNCTION__);
	/*
	syspool_free_size();
	*/
}


#if defined __cplusplus
}
#endif /* __cplusplus */

#endif /* LOG_H__ */
