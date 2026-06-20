#ifndef __BTSPEAKER_TRACE_CFG_H__
#define __BTSPEAKER_TRACE_CFG_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "hal_trace.h"

#define MODULE_TRACE_LEVEL  TR_LEVEL_DEBUG

/******************************macro defination*****************************/
#define LOG_V(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_VERBOSE) TR_VERBOSE(TR_MOD(BTSPK), "[BTSPEAKER-UX]" str, ##__VA_ARGS__)

#define LOG_D(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_DEBUG) TR_DEBUG(TR_MOD(BTSPK), "[BTSPEAKER-UX]" str, ##__VA_ARGS__)

#define LOG_I(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_INFO) TR_INFO(TR_MOD(BTSPK), "[BTSPEAKER-UX]" str, ##__VA_ARGS__)

#define LOG_W(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_WARN) TR_WARN(TR_MOD(BTSPK), "[BTSPEAKER-UX]" str, ##__VA_ARGS__)

#define LOG_E(str, ...) if (MODULE_TRACE_LEVEL >= TR_LEVEL_ERROR) TR_ERROR(TR_MOD(BTSPK), "[BTSPEAKER-UX]" str, ##__VA_ARGS__)

/******************************type defination******************************/

/****************************function declearation**************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __BTSPEAKER_TRACE_CFG_H__ */

