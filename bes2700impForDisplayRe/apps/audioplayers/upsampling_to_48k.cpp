#if defined(__BT_ANC__) || defined(PSAP_FORCE_STREAM_48K)
#include "plat_types.h"
#include "app_audio.h"

#define US_COEF_NUM (24)

static uint32_t g_upsampling_ratio = 1;

/*
8K -->48K
single phase coef Number:=24,    upsample factor:=6,
      64, -311,  210, -320,  363, -407,  410, -356,  196,  191,-1369,30721, 3861,-2265, 1680,-1321, 1049, -823,  625, -471,  297, -327,   34,   19,
   72, -254,   75, -117,   46,   58, -256,  583,-1141, 2197,-4866,28131,10285,-4611, 2935,-2061, 1489,-1075,  755, -523,  310, -291,  -19,   31,
   65, -175,  -73,   84, -245,  457, -786, 1276,-2040, 3377,-6428,23348,17094,-6200, 3592,-2347, 1588,-1073,  703, -447,  236, -203,  -92,   49,
   49,  -92, -203,  236, -447,  703,-1073, 1588,-2347, 3592,-6200,17094,23348,-6428, 3377,-2040, 1276, -786,  457, -245,   84,  -73, -175,   65,
   31,  -19, -291,  310, -523,  755,-1075, 1489,-2061, 2935,-4611,10285,28131,-4866, 2197,-1141,  583, -256,   58,   46, -117,   75, -254,   72,
   19,   34, -327,  297, -471,  625, -823, 1049,-1321, 1680,-2265, 3861,30721,-1369,  191,  196, -356,  410, -407,  363, -320,  210, -311,   64,
*/

const static short coef_8k_upto_48k[6][US_COEF_NUM]__attribute__((section(".sram_data")))  =
{
    {64, -311,  210, -320,  363, -407,  410, -356,  196,    191,-1369,30721, 3861,-2265, 1680,-1321, 1049, -823,  625, -471,  297, -327,   34,   19},
    {72, -254,  75, -117,   46,   58, -256,  583,-1141, 2197,-4866,28131,10285,-4611, 2935,-2061, 1489,-1075,  755, -523,  310, -291,  -19,   31   },
    {65, -175,  -73,   84, -245,  457, -786, 1276,-2040, 3377,-6428,23348,17094,-6200, 3592,-2347, 1588,-1073,  703, -447,  236, -203,  -92,   49  },
    {49,  -92, -203,  236, -447,  703,-1073, 1588,-2347, 3592,-6200,17094,23348,-6428, 3377,-2040, 1276, -786,  457, -245,  84,  -73, -175,   65  },
    {31,  -19, -291,  310, -523,  755,-1075, 1489,-2061, 2935,-4611,10285,28131,-4866, 2197,-1141,  583, -256,  58,   46, -117,   75, -254,   72  },
    {19,   34, -327,  297, -471,  625, -823, 1049,-1321, 1680,-2265, 3861,30721,-1369,  191,  196, -356,  410, -407,  363, -320,  210, -311,   64 }
};

/*
16K -->48K

single phase coef Number:=24,    upsample factor:=3,
       1, -291,  248, -327,  383, -405,  362, -212, -129,  875,-2948,29344, 7324,-3795, 2603,-1913, 1418,-1031,  722, -478,  292, -220,  -86,   16,
   26, -212,    6,   45, -185,  414, -764, 1290,-2099, 3470,-6431,20320,20320,-6431, 3470,-2099, 1290, -764,  414, -185,   45,    6, -212,   26,
   16,  -86, -220,  292, -478,  722,-1031, 1418,-1913, 2603,-3795, 7324,29344,-2948,  875, -129, -212,  362, -405,  383, -327,  248, -291,    1,
*/

const static short coef_16k_upto_48k[3][US_COEF_NUM]  __attribute__((section(".sram_data"))) =
{
    {1, -291,  248, -327,  383, -405,  362, -212, -129, 875,-2948,29344, 7324,-3795, 2603,-1913, 1418,-1031,  722, -478,  292, -220,  -86,   16},
    {26, -212,   6,   45, -185,  414, -764, 1290,-2099, 3470,-6431,20320,20320,-6431, 3470,-2099, 1290, -764,  414, -185,   45,    6, -212,   26},
    {16,  -86, -220,  292, -478,  722,-1031, 1418,-1913, 2603,-3795, 7324,29344,-2948,  875, -129, -212,  362, -405,  383, -327,  248, -291,    1}
};

static short us_para_lst[US_COEF_NUM-1];

static inline short us_get_coef_para(uint32_t samp_idx, uint32_t coef_idx)
{
    if(g_upsampling_ratio == 6)
        return coef_8k_upto_48k[samp_idx][coef_idx];
    else
        return coef_16k_upto_48k[samp_idx][coef_idx];
}

void us_fir_init(uint32_t upsampling_ratio)
{
    g_upsampling_ratio = upsampling_ratio;
    app_audio_memset_16bit(us_para_lst, 0, sizeof(us_para_lst)/sizeof(short));
}


FRAM_TEXT_LOC
uint32_t us_fir_run(short *src_buf, short *dst_buf, uint32_t in_samp_num)
{
    uint32_t in_idx, samp_idx, coef_idx, real_idx, out_idx;
    int para, out;

    for (in_idx = 0, out_idx = 0; in_idx < in_samp_num; in_idx++) {
        for (samp_idx = 0; samp_idx < g_upsampling_ratio; samp_idx++) {
            out = 0;
            for (coef_idx = 0; coef_idx < US_COEF_NUM; coef_idx++) {
                real_idx = coef_idx + in_idx;
                para = (real_idx < (US_COEF_NUM-1))?us_para_lst[real_idx]:src_buf[real_idx - (US_COEF_NUM-1)];
                out += para * us_get_coef_para(samp_idx,coef_idx);
            }

            dst_buf[out_idx] = (short)(out>>16);
            out_idx++;
        }
    }

    if (in_samp_num >= (US_COEF_NUM-1)) {
        app_audio_memcpy_16bit(us_para_lst,
                               (src_buf+in_samp_num-US_COEF_NUM+1),
                               (US_COEF_NUM-1));
    } else {
        uint32_t start_idx = (US_COEF_NUM-1-in_samp_num);

        app_audio_memcpy_16bit(us_para_lst,
                               (us_para_lst+in_samp_num),
                               start_idx);

        app_audio_memcpy_16bit((us_para_lst + start_idx),
                               src_buf,
                               in_samp_num);
    }

    return out_idx;
}

uint32_t voicebtpcm_pcm_resample (short* src_samp_buf, uint32_t src_smpl_cnt, short *dst_samp_buf)
{
    return us_fir_run (src_samp_buf, dst_samp_buf, src_smpl_cnt);
}

#endif
