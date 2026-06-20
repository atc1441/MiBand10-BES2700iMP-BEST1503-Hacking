#pragma once
//
// Copyright 2020 BES Technic
//
// C-preprocessor iterator and list abstraction.

#ifndef _BECO_DEBUG_H
#define _BECO_DEBUG_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

void beco_debug_acc_dump(int acc);
void print_vector32_hex(const uint32_t *tm, int w, int h, size_t stride);

BECO_C_DECLARATIONS_END

#endif
