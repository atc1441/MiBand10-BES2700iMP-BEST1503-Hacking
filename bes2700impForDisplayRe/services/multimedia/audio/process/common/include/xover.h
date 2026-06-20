
#ifndef XOVER_H
#define XOVER_H

#include "iirfilt.h"

struct XoverState_;

typedef struct XoverState_ XoverState;

#ifdef __cplusplus
extern "C" {
#endif

XoverState *crossover_design(int fs, int f0, int size);

void crossover_destroy(XoverState *st);

void crossover_process(XoverState *st, int16_t *ibuf, int16_t *obuf_low, int16_t *obuf_high, int frame_size);

void crossover_process_float(XoverState *st, float *ibuf, float *obuf_low, float *obuf_high, int frame_size);

#ifdef __cplusplus
}
#endif

#endif
