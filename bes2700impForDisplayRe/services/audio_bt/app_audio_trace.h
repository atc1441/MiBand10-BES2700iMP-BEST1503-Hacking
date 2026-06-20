/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __APP_BT_TRACE_H__
#define __APP_BT_TRACE_H__

#include "hal_trace.h"

#ifdef ENABLE_COMPRESS_LOG
#define TRACE_AUD_MGR_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[AUD][MGR]"str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_W(str, ...) TR_WARN(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[AUD][MGR][WARN]"str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_E(str, ...) TR_ERROR(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),    "[AUD][MGR][ERR]"str, ##__VA_ARGS__)
#else
#define TRACE_AUD_MGR_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_I(str, ...) TR_INFO(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_W(str, ...) TR_WARN(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_MGR_E(str, ...) TR_ERROR(TR_MOD(AUDFLG),    str, ##__VA_ARGS__)
#endif

#ifdef ENABLE_COMPRESS_LOG
#define TRACE_AUD_HDL_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[AUD][HDL]"str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_W(str, ...) TR_WARN(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     "[AUD][HDL][WARN]"str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_E(str, ...) TR_ERROR(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),    "[AUD][HDL][ERR]"str, ##__VA_ARGS__)
#else
#define TRACE_AUD_HDL_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_I(str, ...) TR_INFO(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_W(str, ...) TR_WARN(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_HDL_E(str, ...) TR_ERROR(TR_MOD(AUDFLG),    str, ##__VA_ARGS__)
#endif

#ifdef ENABLE_COMPRESS_LOG
#define TRACE_AUD_STREAM_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_W(str, ...) TR_WARN(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_E(str, ...) TR_ERROR(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),    str, ##__VA_ARGS__)
#else
#define TRACE_AUD_STREAM_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_I(str, ...) TR_INFO(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_W(str, ...) TR_WARN(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_AUD_STREAM_E(str, ...) TR_ERROR(TR_MOD(AUDFLG),    str, ##__VA_ARGS__)
#endif

#ifdef ENABLE_COMPRESS_LOG
#define TRACE_MEDIA_PLAYER_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_I(str, ...) TR_INFO(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_W(str, ...) TR_WARN(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),     str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_E(str, ...) TR_ERROR(TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)),    str, ##__VA_ARGS__)
#else
#define TRACE_MEDIA_PLAYESTREAM_D(str, ...) hal_trace_dummy(str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_I(str, ...) TR_INFO(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_W(str, ...) TR_WARN(TR_MOD(AUDFLG),     str, ##__VA_ARGS__)
#define TRACE_MEDIA_PLAYESTREAM_E(str, ...) TR_ERROR(TR_MOD(AUDFLG),    str, ##__VA_ARGS__)
#endif

#endif
