//
// Copyright 2020 BES Technic
//
// Fast high quality random.

#include "beco_rand64.h"

// High quality random
//
static uint64_t random_seed = 1;

static uint64_t xorshift64s(uint64_t *state_a)
{
    uint64_t x = *state_a;    /* The state must be seeded with a nonzero value. */
    x ^= x >> 12; // a
    x ^= x << 25; // b
    x ^= x >> 27; // c
    *state_a = x;
    return x * UINT64_C(0x2545F4914F6CDD1D);
}


void beco_set_random_seed(uint64_t seed)
{
    random_seed = seed ^ 0xfbaf7d4a727d36f1;
}

uint64_t beco_rand64(void)
{
       return xorshift64s(&random_seed);
}

void beco_vec_fill_random(uint64_t *p, int n)
{
    int i;
    for(i = 0; i < n; i++) {
        *p++ = beco_rand64();
    }
}

