// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include "stdint.h"
#include "cmsis.h"
#include "hal_trace.h"

#define MULTI_HEAP_PRINTF hal_trace_printf
#define MULTI_HEAP_STDERR_PRINTF(num,MSG, ...) hal_trace_printf(num,MSG, __VA_ARGS__)
#define MULTI_HEAP_LOCK(PLOCK)  ({uint32_t x = 0; if((PLOCK) != NULL) { x = int_lock();} x;})
#define MULTI_HEAP_UNLOCK(PLOCK, flags) do {if((PLOCK) != NULL) { int_unlock(flags);}} while(0)

#define MULTI_HEAP_ASSERT(CONDITION, ADDRESS) do {if (!(CONDITION)) {ASSERT(0, "Heap corrupt: %p", ADDRESS);} } while (0)

#define MULTI_HEAP_BLOCK_OWNER
#define MULTI_HEAP_SET_BLOCK_OWNER(HEAD)
#define MULTI_HEAP_GET_BLOCK_OWNER(HEAD) (NULL)
