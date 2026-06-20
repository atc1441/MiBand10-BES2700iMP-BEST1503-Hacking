/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#pragma once

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif


EXTERNC int pcm_interlace(short*ch0,short*ch1,int samples,short*output);
EXTERNC int pcm_deinterlace(short* input,int size,short*ch0,short*ch1);

typedef enum{
	PCM_CHANNEL_SELECT_LCHNL,
	PCM_CHANNEL_SELECT_RCHNL,
	PCM_CHANNEL_SELECT_LRMERGE,
	PCM_CHANNEL_SELECT_STEREO,
}pcm_chmode_e;
EXTERNC int pcm_interlace_with_select(short*ch0,short*ch1,int samples,short*output,pcm_chmode_e chmod);
