#ifndef __PSAP_UTILS_H__
#define __PSAP_UTILS_H__
#include "stdint.h"
#include "aud_section.h"

#ifdef __cplusplus
extern "C" {
#endif

/* test code: using this struct as test input */

typedef struct {
    float cs;
    float ct;
    float ws;
    float wt;
    float es;
    float et;
    float at;
    float rt;
    float ta;
    // float cwidth;
    // float din_width;
    // float din_frac;
    // float z_scale;
} psap_cpd_user_cfg;

psap_cpd_config psap_cpd_convert_cfg(const psap_cpd_user_cfg *cfg);

#ifdef __cplusplus
}
#endif

#endif