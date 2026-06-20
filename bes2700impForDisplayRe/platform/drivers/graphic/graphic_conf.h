
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

/**
 * DOC: The LCDC Board Related Configuration.
 *
 */

#ifndef _GRAPHIC_CONF_H
#define _GRAPHIC_CONF_H

//#define BOARD_NAME   best1600_evb

#define TOSTR(s)     #s

#define _LCDC_CONF   _graphic_conf.h
#define _STRCAT(a,b)  a##b
#define STRCAT(a,b)  _STRCAT(a,b)
#define LCDC_CONF_FILE(s) TOSTR(s)

#ifdef BOARD_NAME
#include LCDC_CONF_FILE(STRCAT(BOARD_NAME,_LCDC_CONF))
#else
#include "default_graphic_conf.h"
#endif

#endif //_GRAPHIC_CONF_H
