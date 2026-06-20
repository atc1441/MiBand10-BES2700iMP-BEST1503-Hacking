#ifndef __SPEECH_DEREVERATION2_H__
#define __SPEECH_DEREVERATION2_H__

#include <stdint.h>

/* recommended para
const SpeechDereverberation2Config derevb2_cfg = {
    .bypass =     0,
    .lamda  =     0.99,
    .Lc =         30,
    .ch_num =     2,
    .ola_fac =    3, //only support 1 or 3
};
*/

typedef struct
{
    int32_t     bypass;  //0
    float       lamda;   //0.99
    int32_t     Lc;      //30
    int32_t     ch_num;  //2 for 2 MIC
    int32_t     ola_fac; //3
} SpeechDereverberation2Config;

typedef struct
{
    float re;
    float im;
}cpxdata_t;

#ifdef __cplusplus
extern "C" {
#endif
struct SpeechDereverberation2State_;
typedef struct SpeechDereverberation2State_ SpeechDereverberation2State;
SpeechDereverberation2State *speech_dereverberation2_create(int sample_rate, int frame_size, const SpeechDereverberation2Config *cfg);
int32_t speech_dereverberation2_destroy(SpeechDereverberation2State *st);
int32_t speech_dereverberation2_run(SpeechDereverberation2State *st, short *pcm_buf, int32_t pcm_len, short *out_buf);
#ifdef __cplusplus
}
#endif

#endif