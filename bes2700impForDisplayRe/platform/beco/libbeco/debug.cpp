// Copyright 2020 BES Technic
//
// Description: Debug functions.
//
#include <stdio.h>
#include "beco.h"
#include "beco_read.h"
#include "beco_debug.h"


static void beco_dump_raw_acc(int acc, beco_vec32_out_t *res)
{
    switch(acc){
    case 0: beco_read_acc0_4x4(res, 4); break;
    case 1: beco_read_acc1_4x4(res, 4); break;
    case 2: beco_read_acc2_4x4(res, 4); break;
    case 3: beco_read_acc3_4x4(res, 4); break;
    }
}

void beco_debug_acc_dump(int acc)
{
    beco_vec32_out_t res[4*4];

    uint32_t config, rot;

    // Save current config:
    config = beco_config();

    beco_set_aluconfig(BECO_ALUCNF_BMODE_MAT |
                       BECO_ALUCNF_AMODE_REP32 |
                       BECO_ALUCNF_ATYPE_UINT8 |
                       BECO_ALUCNF_BTYPE_UINT8);
    beco_set_outconfig(BECO_OUTCNF_PACK_INT32);

    beco_dump_raw_acc(acc, res);
    printf("Internal ACC Matrix\n");
    printf("   %8s %8s | %8s %8s\n", "0","1","4","5");
    printf(" 0 %08x %08x | %08x %08x\n", res[0].i32, res[1].i32, res[2].i32, res[3].i32);
    printf(" 2 %08x %08x | %08x %08x\n", res[4].i32, res[5].i32, res[6].i32, res[7].i32);
    printf(" 8 %08x %08x | %08x %08x\n", res[8].i32, res[9].i32, res[10].i32, res[11].i32);
    printf("12 %08x %08x | %08x %08x\n", res[12].i32, res[13].i32, res[14].i32, res[15].i32);
/*
    beco_set_outconfig(BECO_OUTCNF_PACK_INT32);
    beco_dump_raw_acc(acc, res);
    printf("Internal ACC Matrix 16\n");
    printf("   %8s %8s | %8s %8s\n", "0","1","4","5");
    printf(" 0 %08x %08x | %08x %08x\n", res[0].i16[0], res[0].i16[1], res[2].i16[0], res[2].i16[1]);
    printf(" 2 %08x %08x | %08x %08x\n", res[4].i32, res[4].i32, res[6].i32, res[6].i32);
    printf(" 8 %08x %08x | %08x %08x\n", res[8].i32, res[8].i32, res[10].i32, res[10].i32);
    printf("12 %08x %08x | %08x %08x\n", res[12].i32, res[12].i32, res[14].i32, res[14].i32);
*/

    rot = (config & (BECO_CONF_LOCALROTATE | BECO_CONF_GLOBALROTATE));
    if (rot != 0) {
        // Set config with rotation options from parent:
        beco_write_config(BECO_OUTCNF_PACK_INT32 | rot);

        beco_dump_raw_acc(acc, res);

        printf("\nACC Matrix after rotation - ");
        if (rot & BECO_CONF_LOCALROTATE) printf("LOCAL ");
        if (rot & BECO_CONF_GLOBALROTATE) printf("GLOBAL ");
        printf("\n");
        printf("   %8s %8s | %8s %8s\n", "0","1","4","5");
        printf(" 0 %08x %08x | %08x %08x\n", res[0].i32, res[1].i32, res[2].i32, res[3].i32);
        printf(" 2 %08x %08x | %08x %08x\n", res[4].i32, res[5].i32, res[6].i32, res[7].i32);
        printf(" 8 %08x %08x | %08x %08x\n", res[8].i32, res[9].i32, res[10].i32, res[11].i32);
        printf("12 %08x %08x | %08x %08x\n", res[12].i32, res[13].i32, res[14].i32, res[15].i32);
    }
//    if (BECO_ACC_IS_SIGNED(config)
//    float r = (res[4].i32*256 + res0].i32)/128.0/128.0);

    // restore previous config
    beco_write_config(config);
}



void print_vector32_hex(const uint32_t *tm, int w, int h, size_t stride)
{
    int i, j;
    stride = stride*sizeof(beco_vec32_out_t) / sizeof(uint32_t);

    for (i = 0; i < h; i++) {
        for (j = w - 1; j >= 0; j--) {
            printf("%8x ", tm[i*stride + j]);
        }
        printf("\n");
    }
}

