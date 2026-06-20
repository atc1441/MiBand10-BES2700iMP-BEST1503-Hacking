#ifndef __SMF_PROCESS_PLC_H_
#define __SMF_PROCESS_PLC_H_
#include "smf_api.h"

#define MAX_TRACK_NUM 3
typedef struct smf_mix_open_param_t{
    ////////////////////
    int            targetsampleBits;
    unsigned char  trackNum;
    unsigned short trackgain[MAX_TRACK_NUM];  // 0 - 65535
    void*          other;
}smf_plc_open_param_t;
#endif 