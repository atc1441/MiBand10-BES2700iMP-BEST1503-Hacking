#pragma once
//
// Copyright 2020 BES Technic
//
// Fast high quality random.

#ifndef _BECO_RAND64_H
#define _BECO_RAND64_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

void beco_set_random_seed(uint64_t seed);
uint64_t beco_rand64(void);

BECO_C_DECLARATIONS_END

#endif
